/**
 * cwds -- Application-side libcwd support code.
 *
 * @file
 * @brief This file contains the definition of signal_safe_printf.
 *
 * @Copyright (C) 2018  Carlo Wood.
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

#include "signal_safe_printf.h"
#include <cassert>
//#include <cstdio>
#include <cstring>
#include <type_traits>
#include <unistd.h>

template<typename T>
char* signal_safe_write_int(char* buf, unsigned int base, T val)
{
    if (val < 0)
    {
        buf[0] = '-';
        return signal_safe_write_int(buf + 1, base, (typename std::make_unsigned<T>::type(-val)));
    }
    char tmp[64];
    char* p = tmp + sizeof(tmp);
    do
    {
        int digit = val % base;
        val /= base;
        *--p = (digit > 9 ? 'a' - 10 : '0') + digit;
    }
    while (val > 0);
    int size = tmp + sizeof(tmp) - p;
    std::memcpy(buf, p, size);
    return buf + size;
}

void signal_safe_printf(char const* fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    char* out = buf;
    char const* p = fmt;
    while (*p)
    {
        if (*p == '%')
        {
            switch (*++p)
            {
                case '%':           // %%
                {
                    ++p;
                    *out++ = '%';
                    break;
                }
                case 'c':           // %c
                {
                    ++p;
                    char arg = va_arg(args, int);
                    *out++ = arg;
                    break;
                }
                case 'd':           // %d
                {
                    ++p;
                    int arg = va_arg(args, int);
                    out = signal_safe_write_int(out, 10, arg);
                    break;
                }
                case 'l':
                {
                    ++p;
                    if (*p == 'u' || *p == 'x') // %lu and %lx
                    {
                        int base = (*p == 'u') ? 10 : 16;
                        unsigned long arg = va_arg(args, unsigned long);
                        out = signal_safe_write_int(out, base, arg);
                        break;
                    }
                    else if (*p == 'd')         // %ld
                    {
                        long arg = va_arg(args, long);
                        out = signal_safe_write_int(out, 10, arg);
                        break;
                    }
                    *out++ = 'l';
                    break;
                }
                case 's':       // %s
                {
                    ++p;
                    char const* str = va_arg(args, char const*);
                    while (*str) *out++ = *str++;
                    break;
                }
                case 'u':       // %u
                {
                    ++p;
                    unsigned int arg = va_arg(args, unsigned int);
                    out = signal_safe_write_int(out, 10, arg);
                    break;
                }
                case 'x':       // %x
                {
                    ++p;
                    int arg = va_arg(args, int);
                    out = signal_safe_write_int(out, 16, arg);
                    break;
                }
                default:
                    break;
            }
        }
        else
            *out++ = *p++;
    }
    va_end(args);
    size_t s = out - buf;
    assert(s <= sizeof(buf));
    [[maybe_unused]] size_t wlen = write(STDOUT_FILENO, buf, s);
    assert(wlen == s);
}
