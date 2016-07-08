#include "process.h"

// POSIX
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>

// STL
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <climits>

// PDTK
#include "cxxutils/cstringarray.h"

#define assertE(expr) \
  { \
    auto rval = expr; \
    if(!rval) \
    { \
      std::perror("Error"); \
      __assert_fail (#expr, __FILE__, __LINE__, __ASSERT_FUNCTION); \
    } \
  }

Process::Process(void) noexcept
  : m_state(NotStarted),
    m_pid (0),
    m_uid (0),
    m_gid (0),
    m_euid(0),
    m_egid(0),
    m_priority(INT_MAX),
    m_stdout(posix::error_response),
    m_stderr(posix::error_response)
{
  m_arguments.reserve(256);
}

Process::~Process(void) noexcept
{
  m_state = NotStarted;
  m_pid  = 0;
  m_uid  = 0;
  m_gid  = 0;
  m_euid = 0;
  m_egid = 0;

  if(m_stdout != posix::error_response)
    posix::close(m_stdout);
  if(m_stderr != posix::error_response)
    posix::close(m_stderr);
}

bool Process::setWorkingDirectory(const std::string& dir) noexcept
{
  struct stat statbuf;
  if(::stat(dir.c_str(), &statbuf) == posix::success_response)
    m_workingdir = dir;
  return errno == posix::success_response;
}

bool Process::setExecutable(const std::string& executable) noexcept
{
  struct stat statbuf;
  if(::stat(executable.c_str(), &statbuf) == posix::success_response)
  {
    if(m_executable.empty())
      m_arguments.insert(m_arguments.begin(), executable);
    else
      m_arguments.front() = executable;
    m_executable = executable;
  }
  return errno == posix::success_response;
}

void Process::setArguments(const std::vector<std::string>& arguments) noexcept
{
  m_arguments = arguments;
  if(!m_executable.empty())
    m_arguments.insert(m_arguments.begin(), m_executable);
}

bool Process::setUserID(uid_t id) noexcept
{
  if(posix::getpwuid(id) == nullptr)
    return false;
  m_uid = id;
  return true;
}

bool Process::setGroupID(gid_t id) noexcept
{
  if(posix::getgrgid(id) == nullptr)
    return false;
  m_gid = id;
  return true;
}

bool Process::setEffectiveUserID(uid_t id) noexcept
{
  if(posix::getpwuid(id) == nullptr)
    return false;
  m_euid = id;
  return true;
}

bool Process::setEffectiveGroupID(gid_t id) noexcept
{
  if(posix::getgrgid(id) == nullptr)
    return false;
  m_egid = id;
  return true;
}

#ifndef PRIO_MIN
#warning PRIO_MIN is not defined.  Using -20 for value.
#define PRIO_MIN -20
#endif
#ifndef PRIO_MAX
#warning PRIO_MAX is not defined.  Using 20 for value.
#define PRIO_MAX 20
#endif

bool Process::setPriority(int nval) noexcept
{
  if(nval < PRIO_MIN || nval > PRIO_MAX)
    return false;
  m_priority = nval;
  return true;
}

bool Process::start(void) noexcept
{
  if(m_executable.empty())
    return false;

  CStringArray argv(m_arguments);
  CStringArray envv(m_environment, [](const std::pair<std::string, std::string>& p) { return p.first + '=' + p.second; });

  posix::fd_t pipe_stdout[2];
  posix::fd_t pipe_stderr[2];

  m_state = NotStarted;
  m_error = NoError;

  if(::pipe(pipe_stdout) == posix::error_response ||
     ::pipe(pipe_stderr) == posix::error_response ||
     (m_pid = ::fork()) <= posix::error_response)
  {
    m_error = FailedToStart;
    Object::enqueue_copy(error, m_error, static_cast<std::errc>(errno));
    return false;
  }

  m_state = Starting;

  if(m_pid == posix::success_response) // if inside forked process
  {
    // asserts will make it known where it failed via stderr
    assertE(posix::dup2(pipe_stdout[1], STDOUT_FILENO));
    assertE(posix::dup2(pipe_stderr[1], STDERR_FILENO));
    assertE(posix::close(pipe_stdout[0]));
    assertE(posix::close(pipe_stdout[1]));
    assertE(posix::close(pipe_stderr[0]));
    assertE(posix::close(pipe_stderr[1]));

    for(auto& limit : m_limits)
      assertE(::setrlimit(limit.first, &limit.second) != posix::success_response);

    if(m_uid)
      assertE(::setuid(m_uid) == posix::success_response);
    if(m_gid)
      assertE(::setgid(m_gid) == posix::success_response);
    if(m_euid)
      assertE(::seteuid(m_euid) == posix::success_response);
    if(m_egid)
      assertE(::setegid(m_egid) == posix::success_response);

    assertE(::execve(argv[0], argv, envv) == posix::success_response);
    std::perror("execve() implemenation error! This area should be unreachable!");
    assert(false);
  }

  m_stdout = pipe_stdout[0];
  m_stderr = pipe_stderr[0];

  if(!posix::close(pipe_stdout[1]) ||
     !posix::close(pipe_stderr[1]))
  {
    m_error = UnknownError;
    Object::enqueue_copy(error, m_error, static_cast<std::errc>(errno));
    return false;
  }

  if(m_priority >= PRIO_MIN && // only if m_priority has been set
     m_priority <= PRIO_MAX &&
     ::setpriority(PRIO_PROCESS, m_pid, m_priority) != posix::success_response) // set priority
  {
    m_error = UnknownError;
    Object::enqueue_copy(error, m_error, static_cast<std::errc>(errno));
    return false;
  }

  m_state = Running;
  return true;
}

bool Process::sendSignal(posix::signal::EId id, int value) const noexcept
{
  if(m_pid)
    return posix::signal::send(m_pid, id, value);
  return false;
}
