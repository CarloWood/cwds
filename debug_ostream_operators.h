/**
 * cwds -- Application-side libcwd support code.
 *
 * @file
 * @brief This file contains the declaration of debug serializers.
 *
 * @Copyright (C) 2016  Carlo Wood.
 *
 * pub   dsa3072/C155A4EEE4E527A2 2018-08-16 Carlo Wood (CarloWood on Libera) <carlo@alinoe.com>
 * fingerprint: 8020 B266 6305 EE2F D53E  6827 C155 A4EE E4E5 27A2
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

#ifdef CWDEBUG

#include <sys/time.h>
#include <iosfwd>                       // std::ostream&
#include <utility>                      // std::pair
#include <map>
#include <set>
#include <chrono>
#ifdef USE_LIBBOOST
#include <boost/shared_ptr.hpp>         // boost::shared_ptr
#include <boost/weak_ptr.hpp>           // boost::weak_ptr
#endif
#ifndef NAMESPACE_DEBUG
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

#ifdef USE_LIBBOOST
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
#endif // USE_LIBBOOST

namespace std {

/// Print debug info for std::pair&lt;&gt; instance @a data.
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, pair<T1, T2> const& data)
{
  return os << "{first:" << data.first << ", second:" << data.second << '}';
}

/// Print a whole map.
template<typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, map<T1, T2, T3> const& data)
{
  os << "{map<" << NAMESPACE_DEBUG::type_name_of<T1>() <<
      ", " << NAMESPACE_DEBUG::type_name_of<T2>() <<
      ", " << NAMESPACE_DEBUG::type_name_of<T3>() <<">:";
  using map_type = std::map<T1, T2, T3>;
  for (typename map_type::const_iterator iter = data.begin(); iter != data.end(); ++iter)
    os << '{' << *iter << '}';
  return os << '}';
}

/// Print a whole set.
template<typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, set<T1, T2, T3> const& data)
{
  os << "{set<" << NAMESPACE_DEBUG::type_name_of<T1>() <<
      ", " << NAMESPACE_DEBUG::type_name_of<T2>() <<
      ", " << NAMESPACE_DEBUG::type_name_of<T3>() <<">:";
  using set_type = std::set<T1, T2, T3>;
  char const* prefix = "";
  for (typename set_type::const_iterator iter = data.begin(); iter != data.end(); ++iter)
  {
    os << prefix << '{' << *iter << '}';
    prefix = ", ";
  }
  return os << '}';
}

namespace detail {

template <typename T>
concept ConceptNonCharContainer = requires(T v) {
  typename T::value_type;
  typename T::iterator;
  { v.begin() } -> std::same_as<typename T::iterator>;
  { v.end() } -> std::same_as<typename T::iterator>;
  { std::declval<typename T::iterator>() != std::declval<typename T::iterator>() } -> std::same_as<bool>;
  { *std::declval<typename T::iterator>() } -> std::same_as<typename T::value_type&>;
  requires !std::same_as<typename T::value_type, char>; // Exclude containers with value_type char
};

} // namespace detail

/// Print a container.
template<detail::ConceptNonCharContainer CONTAINER>
inline std::ostream& operator<<(std::ostream& os, CONTAINER const& v)
{
  os << '{';
  char const* prefix = "";
  for (auto& val : v)
  {
    if constexpr (std::is_same_v<typename CONTAINER::value_type, bool>)
      os << prefix << std::boolalpha << val;
    else
      os << prefix << val;
    prefix = ", ";
  }
  os << '}';
  return os;
}

template<typename... Args>
std::ostream& operator<<(std::ostream& os, tuple<Args...> const& t)
{
  using ostream_serializer_catch_all::operator<<;
  bool first = true;
  os << "std::tuple<" << ((..., (os << (first ? "" : ", ") << NAMESPACE_DEBUG::type_name_of<Args>(), first = false)), ">(");
  first = true;
  apply([&](auto&&... args){ (..., (os << (first ? "" : ", ") << args, first = false)); }, t);
  return os << ')';
}

// Add support for printing std::u8string to debug output.
std::ostream& operator<<(std::ostream& os, u8string_view utf8_sv);

namespace chrono {

template<std::intmax_t resolution>
std::ostream& operator<<(
    std::ostream& os,
    duration<std::intmax_t, std::ratio<std::intmax_t(1), resolution>> const& duration)
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
std::ostream& operator<<(std::ostream& os, time_point<Clock, Duration> const& timepoint)
{
  return os << timepoint.time_since_epoch();
}

} // namespace chrono

} // namespace std

#endif // CWDEBUG
