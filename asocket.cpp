#include "asocket.h"

// STL
#include <cassert>
#include <cstring>
#include <cstdint>
#include <iostream>

// PDTK
#include "cxxutils/error_helpers.h"
#include "cxxutils/streamcolors.h"

#ifndef CMSG_LEN
#define CMSG_LEN(len) ((CMSG_DATA((struct cmsghdr *) NULL) - (unsigned char *) NULL) + len);

#define CMSG_ALIGN(len) (((len) + sizeof(size_t) - 1) & (size_t) ~ (sizeof(size_t) - 1))
#define CMSG_SPACE(len) (CMSG_ALIGN (len) + CMSG_ALIGN (sizeof (struct cmsghdr)))
#define CMSG_LEN(len)   (CMSG_ALIGN (sizeof (struct cmsghdr)) + (len))
#endif

static_assert(sizeof(uint8_t) == sizeof(char), "size mismatch!");

AsyncSocket::AsyncSocket(void)
  : AsyncSocket(posix::socket(EDomain::unix, EType::stream, EProtocol::unspec, 0))
//  : AsyncSocket(posix::socket(EDomain::unix, EType::datagram, EProtocol::unspec, 0))
{
  m_connected = false;
}

AsyncSocket::AsyncSocket(AsyncSocket& other)
  : AsyncSocket(other.m_read.socket)
{
  m_connected = other.m_connected;
  m_bound     = other.m_bound;
}

AsyncSocket::AsyncSocket(posix::fd_t socket)
{
  m_write.socket = dup(socket);
  m_read .socket = dup(socket);
  posix::close(socket);

  // socket shutdowns do not behave as expected :(
  //shutdown(m_read .socket, SHUT_WR); // make read only
  //shutdown(m_write.socket, SHUT_RD); // make write only

  m_read .thread = std::thread(&AsyncSocket::async_read , this);
  m_write.thread = std::thread(&AsyncSocket::async_write, this);
}

AsyncSocket::~AsyncSocket(void)
{
  if(is_bound() || is_connected())
  {
    posix::close(m_read .socket);
    posix::close(m_write.socket);
  }
}

bool AsyncSocket::bind(const char *socket_path)
{
  if(is_bound() || is_connected())
    return false;

  assert(std::strlen(socket_path) < sizeof(sockaddr_un::sun_path));
  m_addr = socket_path;
  return m_bound = posix::bind(m_read.socket, m_addr, m_addr.size());
}

bool AsyncSocket::listen(int max_connections, std::vector<const char*> allowed_endpoints)
{
  if(!is_bound())
    return false;

  bool ok = posix::listen(m_read.socket, max_connections);
  if(ok)
  {
/*
if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1) {
                        perror("accept");
                        exit(1);
                }
*/

  }
  m_connected = ok;
  return ok;
}

bool AsyncSocket::connect(const char *socket_path)
{
  if(is_connected())
    return false;

  assert(std::strlen(socket_path) < sizeof(sockaddr_un::sun_path));
  m_addr = socket_path;
  m_addr = EDomain::unix;
  return m_connected = posix::connect(m_read.socket, m_addr, m_addr.size());
}

void AsyncSocket::async_read(void)
{
  std::mutex m;
  msghdr msg = {};
  iovec iov = {};

  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  for(;;)
  {
    m_read.buffer.allocate(); // allocate 64KB buffer
    std::unique_lock<std::mutex> lk(m);
    m_read.condition.wait(lk, [this] { return is_connected(); } );

    iov.iov_base = m_read.buffer.data();
    iov.iov_len = m_read.buffer.capacity();
    if(m_read.buffer.expand(posix::recvmsg(m_read.socket, &msg, 0)))
    {
      if(msg.msg_controllen == CMSG_SPACE(sizeof(int)))
      {
        cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
        if(cmsg->cmsg_level == SOL_SOCKET &&
           cmsg->cmsg_type == SCM_RIGHTS &&
           cmsg->cmsg_len == CMSG_LEN(sizeof(int)))
         m_read.fd = *reinterpret_cast<int*>(CMSG_DATA(cmsg));
      }
      else
        m_read.fd = nullptr;
      enqueue<vqueue&, posix::fd_t>(readFinished, m_read.buffer, m_read.fd);
    }
    else // error
      std::cout << std::flush << std::endl << std::red << "error: " << ::strerror(errno) << std::none << std::endl << std::flush;
  }
}

void AsyncSocket::async_write(void)
{
  std::mutex m;
  msghdr msg = {};
  iovec iov = {};
  char aux_buffer[CMSG_SPACE(sizeof(int)) + CMSG_SPACE(sizeof(ucred))] = { 0 };

  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = aux_buffer;

  for(;;)
  {
    std::unique_lock<std::mutex> lk(m);
    m_write.condition.wait(lk, [this] { return is_connected() && !m_write.buffer.empty(); } );

    iov.iov_base = m_write.buffer.begin();
    iov.iov_len = m_write.buffer.size();

    msg.msg_controllen = 0;

    if(m_write.fd != nullptr)
    {
      msg.msg_controllen = CMSG_SPACE(sizeof(int));
      cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
      cmsg->cmsg_level = SOL_SOCKET;
      cmsg->cmsg_type = SCM_RIGHTS;
      cmsg->cmsg_len = CMSG_LEN(sizeof(int));
      *reinterpret_cast<int*>(CMSG_DATA(cmsg)) = m_write.fd;
    }

    int count = posix::sendmsg(m_write.socket, &msg, 0);
    if(count == posix::error_response) // error
      std::cout << std::flush << std::endl << std::red << "error: " << ::strerror(errno) << std::none << std::endl << std::flush;
    else
      enqueue(writeFinished, count);
    m_write.buffer.resize(0);
  }
}

bool AsyncSocket::read(void)
{
  if(!is_connected())
    return false;
  m_read.condition.notify_one();
  return true;
}

bool AsyncSocket::write(vqueue& buffer, posix::fd_t fd)
{
  if(!is_connected())
    return false;

  m_write.fd = fd;
  m_write.buffer = buffer; // share buffer memory
  m_write.condition.notify_one();
  return true;
}
