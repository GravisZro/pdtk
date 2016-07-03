#ifndef APPLICATION_H
#define APPLICATION_H

// STL
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

template<typename T>
struct lockable : T, std::mutex
  { template<typename... ArgTypes> constexpr lockable(ArgTypes... args) noexcept : T(args...) { } };
using vfunc = std::function<void()>;

class Application
{
public:
  Application(void) noexcept;
 ~Application(void) noexcept;

  int exec(void) noexcept;

  static void quit(int return_value = 0) noexcept;

private:
  static std::condition_variable m_step_exec;
  static lockable<std::queue<vfunc>> m_signal_queue;
  friend class Object;
};

#endif // APPLICATION_H
