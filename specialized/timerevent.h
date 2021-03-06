#ifndef TIMEREVENT_H
#define TIMEREVENT_H

// PUT
#include <put/object.h>

template<typename T> constexpr milliseconds_t seconds(T count) { return 1000 * count; }

class TimerEvent : public Object
{
public:
  TimerEvent(void) noexcept;
  ~TimerEvent(void) noexcept;

  bool start(milliseconds_t delay, bool repeat = false) noexcept;
  bool stop(void) noexcept;

  signal<> expired;
private:
  posix::fd_t m_fd;
  struct platform_dependant;
  static struct platform_dependant s_platform;
};

#endif // TIMEREVENT_H
