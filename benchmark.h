/**
 * @file
 * @brief This file contains the declarations of namespace benchmark.
 *
 * Copyright (C) 2018  Carlo Wood.
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

#include <sched.h>
#include <cstdint>
#include <cstdlib>

namespace benchmark {

unsigned int constexpr cache_line_size = 64;    // grep cache_alignment /proc/cpuinfo
unsigned int constexpr number_of_cpus = 8;      // See /proc/cpuinfo

// For this to work reliably, grep '^flags' /proc/cpuinfo must contain rdtscp, constant_tsc and nonstop_tsc.
// You should also turn off all power optimization, Intel Hyper-Threading technology, frequency scaling and
// turbo mode functionalities in the BIOS.
class Stopwatch
{
 private:
  uint32_t cycles_start_high;
  uint32_t cycles_start_low;
  uint32_t cycles_end_high;
  uint32_t cycles_end_low;
  cpu_set_t* m_cpuset;

 public:
  Stopwatch(unsigned int cpu_nr = 0xffffffff);
  ~Stopwatch();

  void* operator new(size_t size);
  void operator delete(void* ptr) { free(ptr); }

  void prefetch()
  {
    __builtin_prefetch(this, 1);
  }

  void start()
  {
    asm volatile (
        "cpuid\n\t"                     // Finish all previous code.
        "rdtsc\n\t"                     // Read Time Stamp Clock register.
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        : "=r" (cycles_start_high), "=r" (cycles_start_low)
        :: "%rax", "%rbx", "%rcx", "%rdx");
  }

  void stop()
  {
    asm volatile (
        "rdtscp\n\t"                    // Read Time Stamp Clock register after all previous code finished.
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        "cpuid\n\t"                     // Prevent subsequent code from being ordered before the read of the Time Stamp Clock.
        : "=r" (cycles_end_high), "=r" (cycles_end_low)
        :: "%rax", "%rbx", "%rcx", "%rdx");
  }

  uint64_t start_cycles() const
  {
    return (uint64_t)cycles_start_high << 32 | cycles_start_low;
  }

  uint64_t stop_cycles() const
  {
    return (uint64_t)cycles_end_high << 32 | cycles_end_low;
  }

  uint64_t diff_cycles() const
  {
    return ((uint64_t)(cycles_end_high - cycles_start_high) << 32) + cycles_end_low - cycles_start_low;
  }
};

} // namespace benchmark
