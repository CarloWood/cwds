#pragma once

// This utility is not threadsafe.

// Usage
//
// Add the following to your .gdbinit file:
#if 0
define rerun_commands
  next 2
  enable
end

define rerun
  set verbose off
  disable
  set $saved_count = restart_dummy_0.count
  tbreak debug::Restart<0>::test_break if debug::Restart<0>::s_count == $saved_count
  commands
    rerun_commands
  end
  set startup-quietly on
  run
end
#endif
//
// Add `RESTART;` at the top of every function that you might want to restart from the top.
//
// Inside gdb, while inside some function, run the command `rerun` and the program will
// run again from the start until it hits the same count of `debug::Restart<0>::s_count`,
// at the top of the current function.
//
// You can also go up the stack first and then type `rerun` to go to the top of that
// function instead.

namespace debug {

template<int Id>
struct Restart
{
  unsigned long count;
  static unsigned long s_count;

  void test_break() { }

  Restart() {
    count = ++s_count;
    test_break();
  }
};

template<int Id>
unsigned long Restart<Id>::s_count;

#define RESTART debug::Restart<0> restart_dummy_0;

} // namespace debug
