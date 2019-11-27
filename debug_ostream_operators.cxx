/**
 * @file
 * @brief This file contains the definition of debug serializers.
 *
 * Copyright (C) 2016, 2017  Carlo Wood.
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

#include <sys.h>        // Put this outside the #ifdef CWDEBUG .. #endif in order
                        // to force recompilation after the configuration changed.

#ifdef CWDEBUG

#include <ostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "debug.h"

/// For debugging purposes. Write a timeval to @a os.
std::ostream& operator<<(std::ostream& os, timeval const& time)
{
  return os << "{tv_sec:" << time.tv_sec << ", tv_usec:" << time.tv_usec << '}';
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

NAMESPACE_DEBUG_END

#endif // CWDEBUG
