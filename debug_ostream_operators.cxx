/**
 * cwds -- Application-side libcwd support code.
 *
 * @file
 * @brief This file contains the definition of debug serializers.
 *
 * @Copyright (C) 2016, 2017  Carlo Wood.
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

#include <sys.h>        // Put this outside the #ifdef CWDEBUG .. #endif in order
                        // to force recompilation after the configuration changed.

#ifdef CWDEBUG

#include <ostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <time.h>
#include "debug.h"

/// For debugging purposes. Write a timeval to @a os.
std::ostream& operator<<(std::ostream& os, timeval const& time)
{
  return os << "{tv_sec:" << time.tv_sec << ", tv_usec:" << time.tv_usec << '}';
}

std::ostream& operator<<(std::ostream& os, tm const& date_time)
{
  return os << "{tm_isdst:" << date_time.tm_isdst << ", tm_yday:" << date_time.tm_yday << ", tm_wday:" << date_time.tm_wday <<
    ", tm_year:" << date_time.tm_year << ", tm_mon:" << date_time.tm_mon << ", tm_mday:" << date_time.tm_mday <<
    ", tm_hour:" << date_time.tm_hour << ", tm_min:" << date_time.tm_min << ", tm_sec:" << date_time.tm_sec << "}";
}

NAMESPACE_DEBUG_START

std::ostream& operator<<(std::ostream& os, PosixMode posix_mode)
{
  int pm = posix_mode.m_posix_mode;

  if ((pm & 3) == 0)
    os << "O_RDONLY";
  else if ((pm & 3) == 1)
    os << "O_WRONLY";
  else if ((pm & 3) == 2)
    os << "O_RDWR";
  else
  {
    os << "<ERROR MODE>";
    return os;
  }
  if ((pm & O_APPEND))
    os << "|O_APPEND";
  if ((pm & O_ASYNC))
    os << "|O_ASYNC";
  if ((pm & O_CLOEXEC))
    os << "|O_CLOEXEC";
  if ((pm & O_CREAT))
    os << "|O_CREAT";
  if ((pm & O_DIRECT))
    os << "|O_DIRECT";
  if ((pm & O_DIRECTORY))
    os << "|O_DIRECTORY";
  if ((pm & O_DSYNC))
    os << "|O_DSYNC";
  if ((pm & O_EXCL))
    os << "|O_EXCL";
  if ((pm & O_LARGEFILE))
    os << "|O_LARGEFILE";
  if ((pm & O_NOATIME))
    os << "|O_NOATIME";
  if ((pm & O_NOCTTY))
    os << "|O_NOCTTY";
  if ((pm & O_NOFOLLOW))
    os << "|O_NOFOLLOW";
  if ((pm & O_NONBLOCK))
    os << "|O_NONBLOCK";
  if ((pm & O_PATH))
    os << "|O_PATH";
  if ((pm & O_SYNC))
    os << "|O_SYNC";
  if ((pm & O_TMPFILE))
    os << "|O_TMPFILE";
  if ((pm & O_TRUNC))
    os << "|O_TRUNC";
  return os;
}

std::ostream& operator<<(std::ostream& os, QuotedString str)
{
  if (!str.m_ptr)
    return os << "nullptr";
  return os << '"' << str.m_ptr << '"';
}

std::ostream& operator<<(std::ostream& os, ArgvList argv)
{
  if (!argv.m_argv)
    return os << "nullptr";
  os << '{';
  int i = 0;
  while (argv.m_argv[i])
  {
    os << '"' << argv.m_argv[i] << "\", ";
    ++i;
  }
  return os << "NULL }";
}

NAMESPACE_DEBUG_END

namespace std {

std::ostream& operator<<(std::ostream& os, std::u8string_view utf8_sv)
{
  os << "u8\"";
  os.write(reinterpret_cast<char const*>(utf8_sv.data()), utf8_sv.length());
  return os << '"';
}

} // namespace std

#endif // CWDEBUG
