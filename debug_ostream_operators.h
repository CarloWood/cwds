/**
 * @file
 * @brief This file contains the declaration of debug serializers.
 *
 * @Copyright (C) 2016 Carlo Wood.
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

#ifdef CWDEBUG

#include <sys/time.h>
#include <iosfwd>                       // std::ostream&
#include <utility>                      // std::pair
#include <map>
#ifdef USE_LIBBOOST
#include <boost/shared_ptr.hpp>         // boost::shared_ptr
#include <boost/weak_ptr.hpp>           // boost::weak_ptr
#endif
#if CWDEBUG_LOCATION
#include <libcwd/type_info.h>
#else
#include <typeinfo>
#endif
#ifndef NAMESPACE_DEBUG
#include <debug.h>                      // NAMESPACE_DEBUG
#endif

NAMESPACE_DEBUG_START

template<typename T>
inline char const* type_name_of()
{
#if CWDEBUG_LOCATION
  return libcwd::type_info_of<T>().demangled_name();
#else
  return typeid(T).name();
#endif
}

/// Use to print human readable form of a POSIX mode (see man 2 open).
struct PosixMode
{
  int m_posix_mode;

  PosixMode(int posix_mode) : m_posix_mode(posix_mode) { }
};

std::ostream& operator<<(std::ostream& os, PosixMode posix_mode);

NAMESPACE_DEBUG_END

struct timeval;

/// Print debug info for timeval instance @a time.
std::ostream& operator<<(std::ostream& os, timeval const& time);

#ifdef USE_LIBBOOST
/// Print debug info for boost::shared_ptr&lt;T&gt;.
template<typename T>
std::ostream& operator<<(std::ostream& os, boost::shared_ptr<T> const& data)
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
std::ostream& operator<<(std::ostream& os, boost::weak_ptr<T> const& data)
{
  return os << "(boost::weak_ptr<" << NAMESPACE_DEBUG::type_name_of<T>() << ">)({ " << *boost::shared_ptr<T>(data) << "})";
}
#endif // USE_LIBBOOST

/// Print debug info for std::pair&lt;&gt; instance @a data.
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, std::pair<T1, T2> const& data)
{
  return os << "{first:" << data.first << ", second:" << data.second << '}';
}

/// Print a whole map.
template<typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, std::map<T1, T2, T3> const& data)
{
  os << "{map<" << NAMESPACE_DEBUG::type_name_of<T1>() <<
      ", " << NAMESPACE_DEBUG::type_name_of<T2>() <<
      ", " << NAMESPACE_DEBUG::type_name_of<T3>() <<">:";
  using map_type = std::map<T1, T2, T3>;
  for (typename map_type::const_iterator iter = data.begin(); iter != data.end(); ++iter)
    os << '{' << *iter << '}';
  return os << '}';
}

#endif // CWDEBUG
