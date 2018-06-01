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

#include "FrequencyCounter.h"
#include <sched.h>
#include <cstdint>
#include <cstdlib>
#include <limits>

#if !defined(__OPTIMIZE__)
#error benchmark.h included without using optimization!
#endif

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
  static int s_stopwatch_overhead;

 public:
  static unsigned int constexpr cpu_any = 0xffffffff;  // This value means: keep running on whatever cpu this thread is running.

  Stopwatch(unsigned int cpu_nr = cpu_any);
  ~Stopwatch();

  void* operator new(size_t size);
  void operator delete(void* ptr) { free(ptr); }

  void prefetch()
  {
    __builtin_prefetch(this, 1);
  }

  // From the Intel instruction set manual,
  // https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html :
  //
  // [Paragraph 8.2.5]
  // ...
  //   The SFENCE, LFENCE, and MFENCE instructions provide a performance-efficient way of ensuring load and store
  //   memory ordering between routines that produce weakly-ordered results and routines that consume that data. The
  //   functions of these instructions are as follows:
  // [...]
  //   LFENCE — Serializes all load (read) operations that occurred prior to the LFENCE instruction in the program
  //   instruction stream, but does not affect store operations.2
  // [...]
  //   2. Specifically, LFENCE does not execute until all prior instructions have completed locally, and no later instruction begins execution
  //   until LFENCE completes. As a result, an instruction that loads from memory and that precedes an LFENCE receives data from memory
  //   prior to completion of the LFENCE. An LFENCE that follows an instruction that stores to memory might complete before the data
  //   being stored have become globally visible. Instructions following an LFENCE may be fetched from memory before the LFENCE, but
  //   they will not execute until the LFENCE completes.
  //
  //
  // [Paragraph 8.3]
  // ...
  //   The Intel 64 and IA-32 architectures define several serializing instructions. These instructions force the
  //   processor to complete all modifications to flags, registers, and memory by previous instructions and to drain all
  //   buffered writes to memory before the next instruction is fetched and executed.
  // [...]
  //   * Non-privileged serializing instructions — CPUID, IRET, and RSM.
  // [...]
  //   When the processor serializes instruction execution, it ensures that all pending memory transactions are
  //   completed (including writes stored in its store buffer) before it executes the next instruction. Nothing can pass a
  //   serializing instruction and a serializing instruction cannot pass any other instruction (read, write, instruction fetch,
  //   or I/O). For example, CPUID can be executed at any privilege level to serialize instruction execution with no effect
  //   on program flow, except that the EAX, EBX, ECX, and EDX registers are modified.
  void start() __attribute__((__always_inline__))
  {
    asm volatile (
        "lfence\n\t"            // Finish all previous code locally, specifically all loads from memory.
                                // Stores to memory might not yet have been flushed to memory (are not visible globally).
        "rdtsc"                 // Only after the LFENCE instruction completes, read the Time Stamp Clock register.
        : "=a" (cycles_start_low), "=d" (cycles_start_high));
  }

  // From Intel® 64 and IA-32 Architectures Developer's Manual,
  // https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-manual-325462.html
  //
  // Vol. 2B 4-547, RDTSCP—Read Time-Stamp Counter and Processor ID
  //
  // The RDTSCP instruction waits until all previous instructions have been executed before reading the counter.
  // However, subsequent instructions may begin execution before the read operation is performed.
  void stop() __attribute__((__always_inline__))
  {
    asm volatile (
        "rdtscp"                // Read Time Stamp Clock register after all previous code finished.
        : "=a" (cycles_end_low), "=d" (cycles_end_high)
        :: "%rcx");
    asm volatile (
        "cpuid"                 // Prevent subsequent code from being ordered before the read of the Time Stamp Clock.
        ::: "%rax", "%rbx", "%rcx", "%rdx");
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

  // Run functor() number_of_runs times and return to smallest interval measured (in cycles).
  template<class T>
  uint64_t get_minimum_of(int const number_of_runs, T const functor)
  {
    uint64_t cycles = std::numeric_limits<uint64_t>::max();
    for (int i = 0; i < number_of_runs; ++i)
    {
      start();
      functor();
      stop();
      uint64_t ncycles = diff_cycles();
      if (ncycles < cycles)
        cycles = ncycles;
    }
    return cycles;
  }

  template<class T>
  eda::FrequencyCounterResult measure(T const functor)
  {
    eda::FrequencyCounter<int, 3> fc;
    while (!fc.add(get_minimum_of(1000, functor)))
      ;
    eda::FrequencyCounterResult result = fc.result();
    result.m_cycles -= s_stopwatch_overhead;
    if (result.m_cycles < 0)
      result.m_cycles = 0;
    return result;
  }

  void calibrate_overhead();
};

} // namespace benchmark
