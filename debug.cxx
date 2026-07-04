// SPDX-FileCopyrightText: 2016-2019, 2021-2022, 2024, 2026 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * cwds -- Application-side libcwd support code.
 *
 * @file
 * @brief This file contains the definitions of debug related objects and functions.
 */

#include <sys.h>                        // Needed for platform-specific code

#ifdef CWDEBUG

#include <cctype>                       // Needed for std::isprint
#include <cstdio>                       // Needed for sprintf
#include <iomanip>                      // Needed for setfill
#include <map>
#include <mutex>
#include <string>
#include <sstream>
#include "debug.h"
#include <unistd.h>                     // Needed for pipe
#ifdef USE_LIBCW
#include <libcw/memleak.h>		// memleak_filter
#endif
#ifdef DEBUGGLOBAL
#include "utils/Singleton.h"            // This header is part of git submodule https://github.com/CarloWood/ai-utils
#endif
#ifdef TRACY_ENABLE
#include <common/TracySystem.hpp>
#endif

#if CWDEBUG_LOCATION

NAMESPACE_DEBUG_START

/**
 * Return call location.
 *
 * @param return_addr The return address of the call.
 */
std::string call_location(void const* return_addr)
{
  libcwd::Location loc((char*)return_addr + libcwd::builtin_return_address_offset);
  std::ostringstream convert;
  convert << loc;
  return convert.str();
}

NAMESPACE_DEBUG_END
#endif // CWDEBUG_LOCATION

NAMESPACE_DEBUG_START

static int s_being_traced = 0;

void ignore_being_traced()
{
  s_being_traced = 1;
}

// Detect if the application is running inside a debugger.
//
// Usage:
//
// #ifdef CWDEBUG
//   if (NAMESPACE_DEBUG::being_traced())
//     DoutFatal(dc::core, "Trap point");
// #endif
//
bool being_traced()
{
  if (s_being_traced == 1)
    return false;

  std::ifstream sf("/proc/self/status");
  std::string s;
  while (sf >> s)
  {
    if (s == "TracerPid:")
    {
      int pid;
      sf >> pid;
      s_being_traced = (pid != 0) ? 2 : 1;
      return pid != 0;
    }
    std::getline(sf, s);
  }

  s_being_traced = 1;
  return false;
}

NAMESPACE_DEBUG_END

HelperPipeFDs::HelperPipeFDs()
{
  if (pipe(m_pipefd) == -1)
  {
    perror("pipe");
    exit(1);
  }
}

std::string DebugPipedOStringStream::str()
{
  std::string result{std::istreambuf_iterator<char>(ibuf()), std::istreambuf_iterator<char>()};
  if (result.back() == '\n')
    result.pop_back();
  return result;
}

NAMESPACE_DEBUG_CHANNELS_START
Channel tracked("TRACKED");
Channel system("SYSTEM");       // Intended to be used for system calls.
Channel restart("RESTART");     // Used by debug::Restart.
NAMESPACE_DEBUG_CHANNELS_END

#endif // CWDEBUG
