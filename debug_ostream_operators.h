// SPDX-FileCopyrightText: 2016-2023, 2025-2026 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * cwds -- Application-side libcwd support code.
 *
 * @file
 * @brief This file contains the declaration of debug serializers.
 */

#pragma once

#ifdef CWDEBUG

#ifndef CWDS_DEBUG_OSTREAM_OPERATORS_H
#define CWDS_DEBUG_OSTREAM_OPERATORS_H

#include <sys/time.h>
#include <iosfwd>                       // std::ostream&
#include <utility>                      // std::pair
#include <map>
#include <set>
#include <chrono>
#include <type_traits>
#ifdef HAVE_LIBBOOST
#include <boost/shared_ptr.hpp>         // boost::shared_ptr
#include <boost/weak_ptr.hpp>           // boost::weak_ptr
#endif
#if !defined(NAMESPACE_DEBUG_START) || !defined(CWDEBUG_LOCATION)
#include <debug.h>                      // NAMESPACE_DEBUG*, CWDEBUG_LOCATION
#endif
#if CWDEBUG_LOCATION
#include <libcwd/type_info.h>
#else
#include <typeinfo>
#endif

NAMESPACE_DEBUG_START

template<typename T>
inline char const* type_name_of()
{
#if CWDEBUG_LOCATION
  return ::libcwd::type_info_of<T>().demangled_name();
#else
  return typeid(T).name();
#endif
}

/// Use to print human readable form of a POSIX mode (see man 2 open).
struct PosixMode
{
  int m_posix_mode;

  explicit PosixMode(int posix_mode) : m_posix_mode(posix_mode) { }
};

std::ostream& operator<<(std::ostream& os, PosixMode posix_mode);

struct QuotedString
{
  char const* m_ptr;
};

std::ostream& operator<<(std::ostream& os, QuotedString str);

// Prints nullptr if str == nullptr, otherwise put double quotes around the string.
inline QuotedString print_string(char const* str)
{
  return { str };
}

struct ArgvList
{
  char const* const* m_argv;
};

std::ostream& operator<<(std::ostream& os, ArgvList argv);

inline ArgvList print_argv(char const* const* argv)
{
  return { argv };
}

NAMESPACE_DEBUG_END

struct timeval;

/// Print debug info for timeval instance @a time.
std::ostream& operator<<(std::ostream& os, timeval const& time);

struct tm;

/// Print debug info for tm instance @a date_time.
std::ostream& operator<<(std::ostream& os, tm const& date_time);

#ifdef HAVE_LIBBOOST
namespace boost {

/// Print debug info for boost::shared_ptr&lt;T&gt;.
template<typename T>
std::ostream& operator<<(std::ostream& os, shared_ptr<T> const& data)
{
  os << "(boost::shared_ptr<" << NAMESPACE_DEBUG::type_name_of<T>() << ">)({";
  if (data.get())
    os << *data;
  else
    os << "<nullptr>";
  return os << "})";
}

/// Print debug info for boost::weak_ptr&lt;T&gt;.
template<typename T>
std::ostream& operator<<(std::ostream& os, weak_ptr<T> const& data)
{
  return os << "(boost::weak_ptr<" << NAMESPACE_DEBUG::type_name_of<T>() << ">)({ " << *shared_ptr<T>(data) << "})";
}

} // namespace boost
#endif // HAVE_LIBBOOST

#ifdef QT_CORE_LIB
#include <QString>
#endif

// This namespace is used in LIBCWD_USING_OSTREAM_PRELUDE.
// Put overloads for types in namespace std here, as defining them in namespace std is UB.
namespace libcwd::ostream_operators {

// Add support for printing std::u8string to debug output.
// Use a template accepting arguments that are convertible to std::u8string_view in order
// to allow an exact definition to take precedence.
std::ostream& operator<<(std::ostream& os, std::convertible_to<std::u8string_view> auto utf8_sv)
{
  os << "u8\"";
  os.write(reinterpret_cast<char const*>(utf8_sv.data()), utf8_sv.length());
  return os << '"';
}

#ifdef QT_CORE_LIB
// Add support for printing QString to debug output.
//std::ostream& operator<<(std::ostream& os, std::convertible_to<QString> auto qstring)
template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, QString> && !std::is_convertible_v<T, std::string>>>
std::ostream& operator<<(std::ostream& os, T const& qstring)
{
  return os << NAMESPACE_DEBUG::print_string(qstring.toUtf8().constData());
}
#endif

template<std::intmax_t resolution>
std::ostream& operator<<(
    std::ostream& os,
    std::chrono::duration<std::intmax_t, std::ratio<std::intmax_t(1), resolution>> const& duration)
{
  std::intmax_t const ticks = duration.count();
  os << (ticks / resolution) << '.';
  std::intmax_t div = resolution;
  std::intmax_t frac = ticks;
  for (;;)
  {
    frac %= div;
    if (frac == 0) break;
    div /= 10;
    os << frac / div;
  }
  return os;
}

template<typename Clock, typename Duration>
std::ostream& operator<<(std::ostream& os, std::chrono::time_point<Clock, Duration> const& timepoint)
{
  return os << timepoint.time_since_epoch();
}

// Forward declarations.
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, std::pair<T1, T2> const& data);

template<typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, std::map<T1, T2, T3> const& data);

template<typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, std::set<T1, T2, T3> const& data);

template<typename... Args>
std::ostream& operator<<(std::ostream& os, std::tuple<Args...> const& t);

#if __cplusplus >= 202002L      // Only add this when C++20 is supported.

namespace detail {

template <typename T>
concept ConceptNonCharContainer = requires(T v)
{
  typename T::value_type;
  typename T::iterator;
  { v.begin() } -> std::same_as<typename T::iterator>;
  { v.end() } -> std::same_as<typename T::iterator>;
  { std::declval<typename T::iterator>() != std::declval<typename T::iterator>() } -> std::same_as<bool>;
  { *std::declval<typename T::iterator>() } -> std::same_as<typename T::value_type&>;
  requires !std::same_as<typename T::value_type, char>;
  requires !std::same_as<typename T::value_type, char8_t>;
#ifdef QT_CORE_LIB
  requires !std::same_as<typename T::value_type, QChar>;
#endif
};

struct ConvertibleFromOstream {
  std::ostream& os_;
  ConvertibleFromOstream(std::ostream& os) : os_(os) { }
};

} // namespace detail

/// Print a container.
template<typename ch, typename char_traits, detail::ConceptNonCharContainer CONTAINER>
inline std::basic_ostream<ch, char_traits>& operator<<(std::basic_ostream<ch, char_traits>& os, CONTAINER const& v)
{
  os << '{';
  char const* prefix = "";
  for (auto& val : v)
  {
    // There is no need to write std::boolalpha here because 1) we should only get here if LIBCWD_USING_OSTREAM_PRELUDE was already used.
    os << prefix;
    LIBCWD_USING_OSTREAM_PRELUDE;
    os << val;
    prefix = ", ";
  }
  os << '}';
  return os;
}

/// Specialization for std::vector<bool>.
template<typename ch, typename char_traits>
inline std::basic_ostream<ch, char_traits>& operator<<(std::basic_ostream<ch, char_traits>& os, std::vector<bool> const& v)
{
  os << '{';
  char const* prefix = "";
  for (bool val : v)
  {
    os << prefix /*<< std::boolalpha*/ << val;
    prefix = ", ";
  }
  os << '}';
  return os;
}

#endif // __cplusplus >= 202002L

/// Print debug info for std::pair&lt;&gt; instance @a data.
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, std::pair<T1, T2> const& data)
{
  LIBCWD_USING_OSTREAM_PRELUDE;
  return os << std::boolalpha << "{first:" << data.first << ", second:" << data.second << '}';
}

/// Print a whole map.
template<typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, std::map<T1, T2, T3> const& data)
{
  os << "{map<" << NAMESPACE_DEBUG::type_name_of<T1>() <<
      ", " << NAMESPACE_DEBUG::type_name_of<T2>() <<
      ", " << NAMESPACE_DEBUG::type_name_of<T3>() <<">:";
  using map_type = std::map<T1, T2, T3>;
  os << std::boolalpha;
  for (typename map_type::const_iterator iter = data.begin(); iter != data.end(); ++iter)
  {
    LIBCWD_USING_OSTREAM_PRELUDE;
    os << *iter;
  }
  return os << '}';
}

/// Print a whole set.
template<typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, std::set<T1, T2, T3> const& data)
{
  os << "{set<" << NAMESPACE_DEBUG::type_name_of<T1>() <<
      ", " << NAMESPACE_DEBUG::type_name_of<T2>() <<
      ", " << NAMESPACE_DEBUG::type_name_of<T3>() <<">:";
  using set_type = std::set<T1, T2, T3>;
  char const* prefix = "";
  os << std::boolalpha;
  for (typename set_type::const_iterator iter = data.begin(); iter != data.end(); ++iter)
  {
    os << prefix;
    LIBCWD_USING_OSTREAM_PRELUDE;
    os << '{' << *iter << '}';
    prefix = ", ";
  }
  return os << '}';
}

template<typename... Args>
std::ostream& operator<<(std::ostream& os, std::tuple<Args...> const& t)
{
  using ostream_serializer_catch_all::operator<<;
  bool first = true;
  os << "std::tuple<" << ((..., (os << (first ? "" : ", ") << NAMESPACE_DEBUG::type_name_of<Args>(), first = false)), ">(");
  first = true;
  os << std::boolalpha;
  LIBCWD_USING_OSTREAM_PRELUDE;
  apply([&](auto&&... args){ (..., (os << (first ? "" : ", ") << args, first = false)); }, t);
  return os << ')';
}

} // libcwd::debug_ostream_operators

#endif // CWDS_DEBUG_OSTREAM_OPERATORS_H
#endif // CWDEBUG
