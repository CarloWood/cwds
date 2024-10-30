#pragma once

#include "debug.h"

// This utility is not threadsafe.

// Usage
//
// Add the following to your .gdbinit file:
/*

# Set the environment variable REPOBASE to point to the directory that contains cwds (the root of the project).
shell echo "set \$REPOBASE = \"$REPOBASE\"" > /tmp/REPOBASE_gdb_command
source /tmp/REPOBASE_gdb_command
eval "source %s/cwds/Restart.gdbinit", $REPOBASE

*/
// Or add cwds/Restart.gdbinit to your .gdbinit file.
//
// Add `RESTART;` at the top of every function that you might want to restart from the top.
//
// Inside gdb, while inside some function, run the command `rerun` and the program will
// run again from the start until it hits the same count of `debug::Restart<0>::s_count`,
// at the top of the current function.
//
// You can also go up the stack first and then type `rerun` to go to the top of that
// function instead.
//
// Alternatively, you can run `rerun <count>` where <count> is the value of `debug::Restart<0>::s_count`
// at which you want to stop.

NAMESPACE_DEBUG_CHANNELS_START
extern channel_ct restart;
NAMESPACE_DEBUG_CHANNELS_END

namespace debug {

template<int Id>
struct Restart
{
  unsigned long count;
  static unsigned long s_count;
  static bool s_restarting;

  void test_break() { }

  Restart() {
    count = ++s_count;
    Dout(dc::restart, "Restart::s_count = " << count);
    test_break();
  }
};

template<int Id>
unsigned long Restart<Id>::s_count;

template<int Id>
bool Restart<Id>::s_restarting;

#define RESTART debug::Restart<0> restart_dummy_0;

// Instantiate s_restarting.
template bool Restart<0>::s_restarting;

} // namespace debug
