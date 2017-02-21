/**
 * @file
 * @brief This file contains the declaration of debug related macros, objects and functions.
 *
 * Copyright (C) 2016 - 2017 Carlo Wood
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifndef CWDEBUG

// No need to document this.  See http://libcwd.sourceforge.net/ for more info.
/// @cond Doxygen_Suppress

#include <iostream>
#include <cstdlib>      // std::exit, EXIT_FAILURE

#define AllocTag1(p)
#define AllocTag2(p, desc)
#define AllocTag_dynamic_description(p, x)
#define AllocTag(p, x)
#define Debug(x) do { } while(0)
#define Dout(a, b)
#define DoutEntering(a, b)
#define DoutFatal(a, b) LibcwDoutFatal(::std, , a, b)
#define ForAllDebugChannels(STATEMENT)
#define ForAllDebugObjects(STATEMENT)
#define LibcwDebug(dc_namespace, x)
#define LibcwDout(a, b, c, d)
#define LibcwDoutFatal(a, b, c, d) do { ::std::cerr << d << ::std::endl; ::std::exit(EXIT_FAILURE); } while (1)
#define NEW(x) new x
#define CWDEBUG_ALLOC 0
#define CWDEBUG_MAGIC 0
#define CWDEBUG_LOCATION 0
#define CWDEBUG_LIBBFD 0
#define CWDEBUG_DEBUG 0
#define CWDEBUG_DEBUGOUTPUT 0
#define CWDEBUG_DEBUGM 0
#define CWDEBUG_DEBUGT 0
#define CWDEBUG_MARKER 0

/// @endcond

#define DEBUG_ONLY(...)
#define COMMA_DEBUG_ONLY(...)

#include <cassert>
#ifdef DEBUG
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif

#else // CWDEBUG

//! Assert \a x, if debugging is turned on.
#define ASSERT(x) LIBCWD_ASSERT(x)

//! Insert debug code, only when debugging.
#define DEBUG_ONLY(...) __VA_ARGS__

//! Insert a comma followed by debug code, only when debugging.
#define COMMA_DEBUG_ONLY(...) , __VA_ARGS__

#ifndef NAMESPACE_DEBUG
#define NAMESPACE_DEBUG debug
#define NAMESPACE_DEBUG_START namespace debug {
#define NAMESPACE_DEBUG_END }
#endif

#ifndef NAMESPACE_CHANNELS
#define NAMESPACE_CHANNELS channels
#endif

#ifndef DEBUGCHANNELS
//! @brief The namespace in which the \c dc namespace is declared.
//
// <A HREF="http://libcwd.sourceforge.net/">Libcwd</A> demands that this macro is defined
// before <libcwd/debug.h> is included and must be the name of the namespace containing
// the \c dc (Debug Channels) namespace.
//
// @sa debug::channels::dc
#define DEBUGCHANNELS ::NAMESPACE_DEBUG::NAMESPACE_CHANNELS
#endif

#include <libcwd/debug.h>

#define NAMESPACE_DEBUG_CHANNELS_START NAMESPACE_DEBUG_START namespace NAMESPACE_CHANNELS { namespace dc {
#define NAMESPACE_DEBUG_CHANNELS_END } } NAMESPACE_DEBUG_END

//! Debug specific code.
NAMESPACE_DEBUG_START

void init();                // Initialize debugging code, called once from main.
void init_thread();         // Initialize debugging code, called once for each thread.

//! @brief Debug Channels (dc) namespace.
//
// @sa debug::channels::dc
namespace NAMESPACE_CHANNELS {

//! The namespace containing the actual debug channels.
namespace dc {
using namespace libcwd::channels::dc;
using libcwd::channel_ct;

// Add the declaration of new debug channels here
// and add their definition in a custom debug.cpp file.
extern channel_ct xmlparser;
extern channel_ct profiler;

} // namespace dc
} // namespace NAMESPACE_CHANNELS

#if CWDEBUG_LOCATION
std::string call_location(void const* return_addr);
#endif

//! @brief Interface for marking scopes of invisible memory allocations.
//
// Creation of the object does nothing, you have to explicitly call
// InvisibleAllocations::on.  Destruction of the object automatically
// cancels any call to \c on of this object.  This makes it exception-
// (stack unwinding) and recursive-safe.
struct InvisibleAllocations {
  int M_on;             //!< The number of times that InvisibleAllocations::on() was called.
  //! Constructor.
  InvisibleAllocations() : M_on(0) { }
  //! Destructor.
  ~InvisibleAllocations() { while (M_on > 0) off(); }
  //! Set invisible allocations on. Can be called recursively.
  void on() { libcwd::set_invisible_on(); ++M_on; }
  //! Cancel one call to on().
  void off() { assert(M_on > 0); --M_on; libcwd::set_invisible_off(); }
};

//! @brief Interface for marking scopes with indented debug output.
//
// Creation of the object increments the debug indentation. Destruction
// of the object automatically decrements the indentation again.
struct Indent {
  int M_indent;                 //!< The extra number of spaces that were added to the indentation.
  //! Construct an Indent object.
  Indent(int indent) : M_indent(indent) { if (M_indent > 0) libcwd::libcw_do.inc_indent(M_indent); }
  //! Destructor.
  ~Indent() { if (M_indent > 0) libcwd::libcw_do.dec_indent(M_indent); }
};

NAMESPACE_DEBUG_END

//! @brief A debug streambuf that prints characters written to it with a green background.
class DebugBuf : public std::streambuf
{
  public:
    DebugBuf() { Dout(dc::notice|continued_cf, ""); setp(0, 0); }
    ~DebugBuf() { Dout(dc::finish, ""); }

    /// Implement std::streambuf::overflow.
    /*virtual*/ int_type overflow(int_type c = traits_type::eof())
    {
      if (c != traits_type::eof())
      {
	if (c == '\n')
	{
	  Dout(dc::finish, "\e[42m\\n\e[0m");
	  Dout(dc::notice|continued_cf, "");
	}
	else
	{
	  Dout(dc::continued, "\e[42m" << (char)c << "\e[0m");
	}
      }
      return c;
    }
};

#if LIBCWD_THREAD_SAFE
extern pthread_mutex_t cout_mutex;
#endif

//! Debugging macro.
//
// Print "Entering " << \a data to channel \a cntrl and increment
// debugging output indentation until the end of the current scope.
#define DoutEntering(cntrl, data) \
  int __cwds_debug_indentation = 2;                                                                      \
  {                                                                                                                     \
    LIBCWD_TSD_DECLARATION;                                                                                             \
    if (LIBCWD_DO_TSD_MEMBER_OFF(::libcwd::libcw_do) < 0)                                                               \
    {                                                                                                                   \
      ::libcwd::channel_set_bootstrap_st __libcwd_channel_set(LIBCWD_DO_TSD(::libcwd::libcw_do) LIBCWD_COMMA_TSD);      \
      bool on;                                                                                                          \
      {                                                                                                                 \
        using namespace LIBCWD_DEBUGCHANNELS;                                                                           \
        on = (__libcwd_channel_set|cntrl).on;                                                                           \
      }                                                                                                                 \
      if (on)                                                                                                           \
        Dout(cntrl, "Entering " << data);                                                                               \
      else                                                                                                              \
        __cwds_debug_indentation = 0;                                                                    \
    }                                                                                                                   \
  }                                                                                                                     \
  debug::Indent __cwds_debug_indent(__cwds_debug_indentation);

#endif // CWDEBUG

#include "debug_ostream_operators.h"
