/**
 * cwds -- Application-side libcwd support code.
 *
 * @file
 * @brief This file contains the declaration of debug related macros, objects and functions.
 *
 * @Copyright (C) 2016, 2017  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This file is part of cwds.
 *
 * Cwds is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cwds is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with cwds.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifndef CWDEBUG

// No need to document this.  See http://carlowood.github.io/libcwd/ for more info.
/// @cond Doxygen_Suppress

#include <iostream>
#include <cstdlib>              // std::exit, EXIT_FAILURE

#define AllocTag1(p)
#define AllocTag2(p, desc)
#define AllocTag_dynamic_description(p, x)
#define AllocTag(p, x)
#define Debug(x) do { } while(0)
#define Dout(a, b) do { } while(0)
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

/// Remove arguments of these macros when CWDEBUG is not defined.
#define CWDEBUG_ONLY(...)
#define COMMA_CWDEBUG_ONLY(...)

#ifdef DEBUG
/// Define this macro as 1 when either CWDEBUG or DEBUG is defined, otherwise as 0.
#define CW_DEBUG 1

#include <cassert>
#define ASSERT(x) assert(x)
#define AI_NEVER_REACHED do { assert(false); __builtin_unreachable(); } while(0);
#define AI_REACHED_ONCE do { static std::atomic_flag s_reached = ATOMIC_FLAG_INIT; assert(!s_reached.test_and_set(std::memory_order_relaxed)); } while(0)

#else
#define CW_DEBUG 0
#define ASSERT(x) do { } while(0)
#define AI_NEVER_REACHED __builtin_unreachable();
#define AI_REACHED_ONCE do { } while(0)

#endif

#else // CWDEBUG

#include <ext/stdio_filebuf.h>  // __gnu_cxx::stdio_filebuf.

/// Define this macro as 1 when either CWDEBUG or DEBUG is defined, otherwise as 0.
#define CW_DEBUG 1

/// Assert @a x, if debugging is turned on.
#define ASSERT(x) LIBCWD_ASSERT(x)
#define AI_NEVER_REACHED do { LIBCWD_ASSERT(false); __builtin_unreachable(); } while(0);
#define AI_REACHED_ONCE do { static std::atomic_flag s_reached = ATOMIC_FLAG_INIT; LIBCWD_ASSERT(!s_reached.test_and_set(std::memory_order_relaxed)); } while(0)

/// Insert debug code, only when debugging.
#define CWDEBUG_ONLY(...) __VA_ARGS__

/// Insert a comma followed by debug code, only when debugging.
#define COMMA_CWDEBUG_ONLY(...) , __VA_ARGS__

#ifndef NAMESPACE_DEBUG
#define NAMESPACE_DEBUG debug
#define NAMESPACE_DEBUG_START namespace debug {
#define NAMESPACE_DEBUG_END }
#endif

#ifndef NAMESPACE_CHANNELS
#define NAMESPACE_CHANNELS channels
#endif

#ifndef DEBUGCHANNELS
/**
 * The namespace in which the @c dc namespace is declared.
 *
 * <A HREF="http://carlowood.github.io/libcwd/">Libcwd</A> demands that this macro is defined
 * before <libcwd/debug.h> is included and must be the name of the namespace containing
 * the @c dc (Debug Channels) namespace.
 *
 * @sa debug::channels::dc
 */
#define DEBUGCHANNELS ::NAMESPACE_DEBUG::NAMESPACE_CHANNELS
#endif

#include <libcwd/debug.h>

#define NAMESPACE_DEBUG_CHANNELS_START NAMESPACE_DEBUG_START namespace NAMESPACE_CHANNELS { namespace dc {
#define NAMESPACE_DEBUG_CHANNELS_END } } NAMESPACE_DEBUG_END

namespace libcwd {

enum thread_init_t {
  thread_init_default,
  from_rcfile,
  copy_from_main,
  debug_off
};

} // namespace libcwd

#include <atomic>       // atomic_bool

/// Debug specific code.
NAMESPACE_DEBUG_START

void init();                                                                            // Initialize debugging code, called once from main.
extern libcwd::thread_init_t thread_init_default;
void init_thread(std::string thread_name = "", libcwd::thread_init_t thread_init = libcwd::thread_init_default);      // Initialize debugging code, called once for each thread.
extern std::atomic_bool threads_created;

/**
 * Debug Channels (dc) namespace.
 *
 * @sa debug::channels::dc
 */
namespace NAMESPACE_CHANNELS {

/// The namespace containing the actual debug channels.
namespace dc {
using namespace libcwd::channels::dc;
using libcwd::channel_ct;

// Add the declaration of new debug channels here
// and add their definition in a custom debug.cpp file.

} // namespace dc
} // namespace NAMESPACE_CHANNELS

#if CWDEBUG_LOCATION
std::string call_location(void const* return_addr);
#endif
bool being_traced();

/**
 * Interface for marking scopes of invisible memory allocations.
 *
 * Creation of the object does nothing, you have to explicitly call
 * InvisibleAllocations::on. Destruction of the object automatically
 * cancels any call to @c{on()} of this object. This makes it exception-
 * (stack unwinding) and recursive-safe.
 */
struct InvisibleAllocations
{
  /// The number of times that InvisibleAllocations::on() was called.
  int M_on;

  /// Constructor.
  InvisibleAllocations() : M_on(0) { }

  /// Destructor.
  ~InvisibleAllocations() { while (M_on > 0) off(); }

  /// Set invisible allocations on. Can be called recursively.
  void on() { libcwd::set_invisible_on(); ++M_on; }

  /// Cancel one call to on().
  void off() { assert(M_on > 0); --M_on; libcwd::set_invisible_off(); }
};

/**
 * Interface for marking scopes with indented debug output.
 *
 * Creation of the object increments the debug indentation. Destruction
 * of the object automatically decrements the indentation again.
 */
struct Indent
{
  /// The extra number of spaces that were added to the indentation.
  int M_indent;

  /// Construct an Indent object.
  explicit Indent(int indent) : M_indent(indent) { if (M_indent > 0) libcwd::libcw_do.inc_indent(M_indent); }

  /// Destructor.
  ~Indent() { if (M_indent > 0) libcwd::libcw_do.dec_indent(M_indent); }
};

/**
 * Interface for marking scopes with a marker character.
 *
 * Creation of the object appends the character and a space to
 * the current marker after first adding the current indentation
 * to it as spaces, and sets the indentation to zero. Destruction
 * restores things again.
 */
struct Mark
{
  /// The old indentation.
  int M_indent;

  /// Construct a Mark object.
  explicit Mark(char m = '|') : M_indent(libcwd::libcw_do.get_indent())
  {
    libcwd::libcw_do.push_marker();
    libcwd::libcw_do.marker().append(std::string(M_indent, ' ') + m + ' ');
    // This is basically a decrement of M_indent.
    libcwd::libcw_do.set_indent(0);
  }
  explicit Mark(char const* utf8_m) : M_indent(libcwd::libcw_do.get_indent())
  {
    libcwd::libcw_do.push_marker();
    libcwd::libcw_do.marker().append(std::string(M_indent, ' ') + utf8_m + ' ');
    // This is basically a decrement of M_indent.
    libcwd::libcw_do.set_indent(0);
  }
#ifdef __cpp_char8_t
  explicit Mark(char8_t const* utf8_m) : Mark(reinterpret_cast<char const*>(utf8_m)) { }
#endif

  /// Destructor.
  ~Mark() { end(); }

  void end()
  {
    if (M_indent != -1)
    {
      libcwd::libcw_do.pop_marker();
      // Restore indentation relative to possible other indentation increments that happened in the mean time.
      libcwd::libcw_do.inc_indent(M_indent);
      // Mark that end() was already called.
      M_indent = -1;
    }
  }
};

NAMESPACE_DEBUG_END

NAMESPACE_DEBUG_CHANNELS_START
extern channel_ct system;
NAMESPACE_DEBUG_CHANNELS_END

/// A debug streambuf that prints characters written to it with a green background.
class DebugBuf : public std::streambuf
{
  public:
    DebugBuf() { Dout(dc::notice|continued_cf, ""); setp(0, 0); }
    ~DebugBuf() { Dout(dc::finish, ""); }

    /// Implement std::streambuf::overflow.
    int_type overflow(int_type c = traits_type::eof()) override
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

/// A class that wraps a POSIX pipe(2). Helper class for DebugPipedOStringStream.
class HelperPipeFDs
{
 private:
  int m_pipefd[2];

 protected:
  HelperPipeFDs();

  int fd_out() const { return m_pipefd[0]; }  // The read end of the pipe.
  int fd_in() const { return m_pipefd[1]; }   // The write end of the pipe.
};

/**
 * A class that wraps two __gnu_cxx::stdio_filebuf<char>'s.
 *
 * Helper class for DebugPipedOStringStream.
 */
struct HelperPipeBufs : public HelperPipeFDs
{
 private:
  __gnu_cxx::stdio_filebuf<char> m_obuf;  // Read from ostream, write to pipe.
  __gnu_cxx::stdio_filebuf<char> m_ibuf;  // Read from pipe write to istream.

 protected:
  HelperPipeBufs() : m_obuf(fd_in(), std::ios::out), m_ibuf(fd_out(), std::ios::in) { }

  std::streambuf* obuf() { return &m_obuf; }
  std::streambuf* ibuf() { return &m_ibuf; }

 public:
  /// Flush and close write-end of pipe. Unblocks DebugPipedOStringStream::str().
  void close() { m_obuf.close(); }
};

class DebugPipedOStringStream : public HelperPipeBufs, public std::ostream
{
 public:
  DebugPipedOStringStream() : std::ostream(obuf()) { }

  /// Read blocking from read-end of pipe until EOF. Call close() (after writing) to unblock.
  std::string str();
};

#if LIBCWD_THREAD_SAFE
namespace libcwd {
extern pthread_mutex_t cout_mutex;
} // namespace libcwd
#endif

/**
 * Debugging macro.
 *
 * Print "Entering " << @a data to channel @a cntrl and increment
 * debugging output indentation until the end of the current scope.
 */
#define DoutEntering(cntrl, data) \
  int __cwds_debug_indentation = 2;                                                                                     \
  {                                                                                                                     \
    LIBCWD_TSD_DECLARATION;                                                                                             \
    if (LIBCWD_DO_TSD_MEMBER_OFF(::libcwd::libcw_do) < 0)                                                               \
    {                                                                                                                   \
      using namespace ::libcwd;                                                                                         \
      ::libcwd::channel_set_bootstrap_st __libcwd_channel_set(LIBCWD_DO_TSD(::libcwd::libcw_do) LIBCWD_COMMA_TSD);      \
      bool on;                                                                                                          \
      {                                                                                                                 \
        using namespace LIBCWD_DEBUGCHANNELS;                                                                           \
        on = (__libcwd_channel_set|cntrl).on;                                                                           \
      }                                                                                                                 \
      if (on)                                                                                                           \
        do                                                                                                              \
        {                                                                                                               \
          LIBCWD_ASSERT_NOT_INTERNAL;                                                                                   \
          LIBCWD_LibcwDoutScopeBegin_MARKER;                                                                            \
          ::libcwd::debug_ct& __libcwd_debug_object(::libcwd::libcw_do);                                                \
          LIBCWD_DO_TSD(__libcwd_debug_object).start(__libcwd_debug_object, __libcwd_channel_set LIBCWD_COMMA_TSD);     \
          LibcwDoutStream << "Entering " << data;                                                                       \
          LIBCWD_DO_TSD(__libcwd_debug_object).finish(__libcwd_debug_object, __libcwd_channel_set LIBCWD_COMMA_TSD);    \
        } while(0);                                                                                                     \
      else                                                                                                              \
        __cwds_debug_indentation = 0;                                                                                   \
    }                                                                                                                   \
  }                                                                                                                     \
  NAMESPACE_DEBUG::Indent __cwds_debug_indent(__cwds_debug_indentation);

#ifdef __cpp_fold_expressions

// Allow printing of template parameter packs.
//
// Usage: Dout(dc::notice, join(" + ", args...));
//
// Which would print, for three arguments: "arg0 + arg1 + arg2".
//

#include <tuple>
#include <type_traits>

template<typename ...Args>
struct Join
{
  char const* m_separator;
  std::tuple<Args const&...> m_args;
  Join(char const* separator, Args const&... args) : m_separator(separator), m_args(args...) { }
  Join(char const* separator, std::tuple<Args const&...>&& args) : m_separator(separator), m_args(std::move(args)) { }
  template<size_t ...I> void print_on(std::ostream& os, std::index_sequence<I...>);
};

namespace ostream_serializer_catch_all {

template<typename, typename = std::ostream&>
struct has_ostream_serializer : std::false_type
{
};

template<typename T>
struct has_ostream_serializer<T, decltype(std::cout << std::declval<T>())> : std::true_type
{
};

template<typename T>
auto operator<<(std::ostream& os, T const&) -> typename std::enable_if<!has_ostream_serializer<T>::value, std::ostream&>::type
{
  os.write("...", 3);
  return os;
}

} // namespace ostream_serializer_catch_all

template<typename ...Args>
template<size_t ...I>
void Join<Args...>::print_on(std::ostream& os, std::index_sequence<I...>)
{
  using ostream_serializer_catch_all::operator<<;
  (..., (os << (I == 0 ? "" : m_separator) << std::get<I>(m_args)));
}

template<typename ...Args>
std::ostream& operator<<(std::ostream& os, Join<Args...> comm)
{
  comm.print_on(os, std::make_index_sequence<sizeof...(Args)>());
  return os;
}

template<typename ...Args>
Join<Args...> join(char const* separator, Args const&... args)
{
  return { separator, args... };
}

template<typename ...Args>
Join<Args...> join_from_tuple(char const* separator, std::tuple<Args...>&& args)
{
  return { separator, std::move(args) };
}

template<typename ...Args>
Join<char const*, Args...> join_more(char const* separator, Args const&... args)
{
  // We must use a static here because the tuple binds with a const& (aka, stores a char const* const& to empty_prefix).
  // Using a string literal directly results in a reference to a temporary char const* pointing to the char[1] of the literal.
  // Thanks to aschepler for this analysis (see https://stackoverflow.com/a/57342183/1487069).
  static char const* const empty_prefix = "";
  return { separator, empty_prefix, args... };
}

#endif // __cpp_fold_expressions

#endif // CWDEBUG

#if CW_DEBUG

#define DEBUG_ONLY(...) __VA_ARGS__
#define COMMA_DEBUG_ONLY(...) , __VA_ARGS__

#else

#define DEBUG_ONLY(...)
#define COMMA_DEBUG_ONLY(...)

#endif

#include "debug_ostream_operators.h"
