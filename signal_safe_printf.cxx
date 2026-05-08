/**
 * cwds -- Application-side libcwd support code.
 *
 * @file
 * @brief This file contains the definition of signal_safe_printf.
 *
 * @Copyright (C) 2018  Carlo Wood.
 *
 * pub   dsa3072/C155A4EEE4E527A2 2018-08-16 Carlo Wood (CarloWood on Libera) <carlo@alinoe.com>
 * fingerprint: 8020 B266 6305 EE2F D53E  6827 C155 A4EE E4E5 27A2
 *
 * This file is part of cwds.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
