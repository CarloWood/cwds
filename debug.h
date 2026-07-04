// SPDX-FileCopyrightText: 2016-2023, 2025-2026 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * cwds -- Application-side libcwd support code.
 *
 * @file
 * @brief This file contains the declaration of debug related macros, objects and functions.
 */

#pragma once

#ifndef CWDEBUG

// No need to document this.  See http://carlowood.github.io/libcwd/ for more info.
/// @cond Doxygen_Suppress

#include <iostream>
#include <cstdlib>              // std::exit, EXIT_FAILURE

#define Debug(...) do { } while(0)
#define Dout(a, ...) do { } while(0)
#define DoutEntering(a, ...)
#define DoutFatal(a, ...) LibcwDoutFatal(::std, , a, __VA_ARGS__)
#define ForAllDebugChannels(...)
#define ForAllDebugObjects(...)
#define LibcwDebug(dc_namespace, ...)
#define LibcwDout(a, b, c, ...)
#define LibcwDoutFatal(a, b, c, ...) do { ::std::cerr << __VA_ARGS__ << ::std::endl; ::std::exit(EXIT_FAILURE); } while (1)
#define CWDEBUG_LOCATION 0
#define CWDEBUG_DEBUG 0
#define CWDEBUG_DEBUGOUTPUT 0
#define CWDEBUG_DEBUGT 0

/// @endcond

/// Remove arguments of these macros when CWDEBUG is not defined.
#define CWDEBUG_ONLY(...)
#define COMMA_CWDEBUG_ONLY(...)

#ifndef NDEBUG
/// Define this macro as 1 when either CWDEBUG is defined or NDEBUG is not defined, otherwise as 0.
#define CW_DEBUG 1

#include <cassert>
#include <atomic>

#define ASSERT(...) assert(__VA_ARGS__)
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#include <utility>
#define AI_NEVER_REACHED do { ::std::unreachable(); } while(0);
#else
#define AI_NEVER_REACHED __builtin_unreachable();
#endif
#define AI_REACHED_ONCE do { \
  static ::std::atomic_flag s_reached = ATOMIC_FLAG_INIT; assert(!s_reached.test_and_set(::std::memory_order_relaxed)); \
} while(0)

#else
#define CW_DEBUG 0
#define ASSERT(...) do { } while(0)
#define AI_NEVER_REACHED __builtin_unreachable();
#define AI_REACHED_ONCE do { } while(0)
#endif

#else // CWDEBUG

#include <ext/stdio_filebuf.h>  // __gnu_cxx::stdio_filebuf.
#include <mutex>

// Added this assert because C++17 doesn't complain if you include <concepts>;
// you just don't get any and C++17 is still the default for clang++!
static_assert(__cplusplus >= 202002L, "cwds requires C++20.");
#include <concepts>

// libcwd version 2 does not define LIBCWD_THREAD_SAFE anymore; it is always thread-safe anyway.
#if defined(LIBCWD_THREAD_SAFE)
#error "This version of cwds doesn't support libcwd version 1. Either downgrade cwds or upgrade libcwd and port your project to libcwd-2."
#endif

/// Assert @a x, if debugging is turned on.
#define ASSERT(...) LIBCWD_ASSERT(__VA_ARGS__)
#define AI_NEVER_REACHED do { LIBCWD_ASSERT(false); __builtin_unreachable(); } while(0);
#define AI_REACHED_ONCE do { static std::atomic_flag s_reached = ATOMIC_FLAG_INIT; LIBCWD_ASSERT(!s_reached.test_and_set(std::memory_order_relaxed)); } while(0)

/// Insert debug code, only when debugging.
#define CWDEBUG_ONLY(...) __VA_ARGS__

/// Insert a comma followed by debug code, only when debugging.
#define COMMA_CWDEBUG_ONLY(...) , __VA_ARGS__

#ifndef LIBCWD_USING_OSTREAM_PRELUDE
struct MakeLIBCWD_USING_OSTREAM_PRELUDEHappy;
namespace libcwd::ostream_operators {
void operator<<(std::same_as<MakeLIBCWD_USING_OSTREAM_PRELUDEHappy> auto, int);
} // namespace libcwd::ostream_operators
#define LIBCWD_USING_OSTREAM_PRELUDE using ::libcwd::ostream_operators::operator<<;
#endif

#include <libcwd/debug.h>
#include <libcwd/char2str.h>

/// Debug specific code.
NAMESPACE_DEBUG_START

#if CWDEBUG_LOCATION
std::string call_location(void const* return_addr);
#endif
bool being_traced();

void ignore_being_traced();

#if __cplusplus >= 202002L      // Only add this when C++20 is supported.

template <typename T>
concept ConceptAlwaysFalse = false;

template <ConceptAlwaysFalse T>
bool static_print(T&& = {})
{
  return false;
};

// Usage:
//
// DEBUG_STATIC_PRINT_TYPE(decltype(Class::element));
//
#define DEBUG_STATIC_PRINT_TYPE(type) \
  auto __dummy = NAMESPACE_DEBUG::static_print<decltype(TopPosition::v)>();

#endif // __cplusplus >= 202002L

NAMESPACE_DEBUG_END

NAMESPACE_DEBUG_CHANNELS_START
extern Channel system;
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
          Dout(dc::finish, "\033[42m\\n\033[0m");
          Dout(dc::notice|continued_cf, "");
        }
        else
        {
          Dout(dc::continued, "\033[42m" << (char)c << "\033[0m");
        }
      }
      return c;
    }
};

/// A debug streambuf that prints characters written to it to a given debug channel.
class DebugStreamBuf : public std::streambuf
{
  private:
    libcwd::Channel const& m_debug_channel;

  public:
    DebugStreamBuf(libcwd::Channel const& debug_channel) : m_debug_channel(debug_channel)
    {
      Dout(debug_channel|continued_cf, "");
      setp(0, 0);
    }

    ~DebugStreamBuf()
    {
      Dout(dc::finish, "");
    }

    /// Implement std::streambuf::overflow.
    int_type overflow(int_type c = traits_type::eof()) override
    {
      if (c != traits_type::eof())
      {
        if (c == '\n')
        {
          Dout(dc::finish, "\\n");
          Dout(m_debug_channel|continued_cf, "");
        }
        else
        {
          Dout(dc::continued, char2str(c));
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

#ifdef __cpp_fold_expressions

// Allow printing of template parameter packs.
//
// Usage: Dout(dc::notice, join(" + ", args...));
//
// Which would print, for three arguments: "arg0 + arg1 + arg2".
//

#include <tuple>
#include <type_traits>
#include <iostream>

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
  os.write("¿￼ ?", 4);
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
