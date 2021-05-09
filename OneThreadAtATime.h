#pragma once

#include "debug.h"

// Usage:
//
// Define the global atomic boolean wrapper:
//
//   OneThreadAtATime critical_area_01;
//
// Also include utils::threading inside utils somewhere
// (or use utils::threading::unlock_guard below):
//
//   namespace utils { using namespace threading; }
//
// Then around each block of this critical area, use:
//
//   {
// #if CW_DEBUG
//     std::lock_guard<OneThreadAtATime> lk(critical_area_01);
// #endif
//     ... critical area here ...
//   }
//
// Or, for example
//
//   {
// #if CW_DEBUG
//     std::unique_lock<OneThreadAtATime> ul(critical_area_01);
// #endif
//     ... critical area here ...
//     {
// #if CW_DEBUG
//       utils::unlock_guard<OneThreadAtATime> unlk(ul);
// #endif
//       ... non-critical area (for example a callback function) ...
//     }
//     ... critical area here ...
//   }

#if CW_DEBUG
#include <atomic>
#include <thread>

class OneThreadAtATime
{
 private:
  std::atomic<std::thread::id> m_owner;
  int m_recursive;

 public:
  OneThreadAtATime() : m_recursive(0) { }

  void lock()
  {
    std::thread::id previous_owner = m_owner.exchange(std::this_thread::get_id(), std::memory_order_relaxed);
    ASSERT(previous_owner == std::thread::id() || previous_owner == std::this_thread::get_id());
    ++m_recursive;
  }

  void unlock()
  {
    if (--m_recursive == 0)
      m_owner.store(std::thread::id(), std::memory_order_relaxed);
  }
};
#endif // CW_DEBUG
