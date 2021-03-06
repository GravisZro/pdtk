#ifndef EVENTBACKEND_H
#define EVENTBACKEND_H

// STL
#include <mutex>
#include <functional>
#include <unordered_map>
#include <list>

// PUT
#include <put/cxxutils/posix_helpers.h>
#include <put/specialized/mutex.h>

typedef uint64_t native_flags_t;
typedef int milliseconds_t;

namespace EventBackend
{
  using callback_t = std::function<void(posix::fd_t, native_flags_t) noexcept>;
  struct callback_info_t
  {
    native_flags_t flags;
    callback_t function;
  };

  extern bool add(posix::fd_t target, native_flags_t flags, callback_t function) noexcept; // add FD to montior
  extern bool remove(posix::fd_t target, native_flags_t flags) noexcept; // remove from watch queue

  extern bool poll(milliseconds_t timeout = -1) noexcept;

  extern posix::lockable<std::unordered_multimap<posix::fd_t, callback_info_t>> queue; // watch queue
  extern std::list<std::pair<posix::fd_t, native_flags_t>> results; // results from getevents()

  extern struct platform_dependant s_platform;
  extern const native_flags_t SimplePollReadFlags;
}

#endif // EVENTBACKEND_H
