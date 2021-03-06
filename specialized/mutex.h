#ifndef MUTEX_H
#define MUTEX_H

#include <put/specialized/osdetect.h>

#if defined(__tru64__) && !defined(FORCE_POSIX_MUTEXES)
# include <tis.h> // Tru64
typedef pthread_mutex_t mutex_type;

#elif defined(__plan9__) && !defined(FORCE_POSIX_MUTEXES)
# include <lock.h> // Plan 9
typedef Lock mutex_type;

#elif defined(_POSIX_THREADS)
# include <pthread.h> // POSIX
typedef pthread_mutex_t mutex_type;

#elif !defined(SINGLE_THREADED_APPLICATION)
# pragma message("DANGER! No mutex facility detected. Multithreaded applications may crash!")
# define SINGLE_THREADED_APPLICATION
#endif

namespace posix
{
#if !defined(SINGLE_THREADED_APPLICATION)
  class mutex
  {
  public:
    mutex(void) noexcept;
    ~mutex(void) noexcept;
    bool lock(void) noexcept;
    bool unlock(void) noexcept;
  private:
    mutex_type m_mutex;
  };
#else
  struct mutex
  {
    constexpr bool lock(void) const noexcept { return true; }
    constexpr bool unlock(void) const noexcept { return true; }
  };
#endif

  template<typename T>
  struct lockable : T, posix::mutex
  {
    template<typename... ArgTypes>
    lockable(ArgTypes... args) noexcept
      : T(args...) { }
  };
}

#endif // MUTEX_H
