/**
 * cwds -- Application-side libcwd support code.
 *
 * @file
 * @brief This file contains the declarations of namespace benchmark.
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

#pragma once

#include "FrequencyCounter.h"
#include <sched.h>
#include <cstdint>
#include <cstdlib>
#include <limits>

#if !defined(__OPTIMIZE__)
#error benchmark.h included without using optimization!
#endif

// Usage example
//
// cpu_frequency is 1 / delta_t where delta_t is the time between
// two clock ticks as measured with rdtsc / rdtscp. It should be
// the cpu clock frequency.
//
// cpu is a number from 0 till N, where N is the number of cores
// you have (see: grep '^processor' /proc/cpuinfo).
//
// loopsize is the size of the loop around the test code.
// It should be such that the result (in clock cycles)
// is between 1000 and 10000. The reason is simply accuracy:
// the measurements are likely to reproduce within a single
// clock cycle, so 1000 is more than enough for accuracy (0.1%)
// while over 10000 just starts to waste too much time without
// any gain (and increasing the chance for interrupts invalidating
// measurements).
//
// minimum_of is the loop size of a loop around the inner loop
// that measures the number of cycles needed for executing the
// test code loopsize times. Only the minimum measurement of
// minimum_of measurements is used. This is an easy way to
// quickly get rid of outliers, so a value of 3 should be enough;
// it is when a value of 4 (etc) doesn't give a better reproducablity
// and/or speed up the total time of the measurement.
//
// nk is the number of buckets of FrequencyCounter to average over.
// The above minimum measurements are put in buckets, if the actual
// variation of the measurements is large than it might makes sense
// to not just report the bucket with the largest count.
// My guess is that a value of 3 should be right for most measurements
// (sharp peaks) while a value of 8 might be needed but might start
// to pick up a different execution paths (ie, a there is a spike
// of measurements of 5 clocks, nearly none of 7 clocks, but then
// there are also a significant number of executions that take 10
// clocks; then a small value of nk will report 5, while a larger
// value will report maybe 7 clocks - the average between the
// two peaks (assuming they have about the same occurance count).
//

#ifdef EXAMPLE_CODE        // Undefined

#include "sys.h"
#include "debug.h"
#include "cwds/benchmark.h"
//#include "iacaMarks.h"        // Optionally include to define IACA_START and IACA_STOP.

double const cpu_frequency = 3612059050.0;        // In cycles per second.
int const cpu = 0;                                // The CPU to run on.
size_t const loopsize = 1000;                     // We'll be measing the number of clock cylces needed for this many iterations of the test code.
size_t const minimum_of = 3;                      // All but the fastest measurement of this many measurements are thrown away (3 is normally enough).
int const nk = 3;                                 // The number of buckets of FrequencyCounter (with the highest counts) that are averaged over.

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  benchmark::Stopwatch stopwatch(cpu);          // Declare stopwatch and configure on which CPU it must run.

  // Calibrate Stopwatch overhead.
  stopwatch.calibrate_overhead(loopsize, minimum_of);

  uint64_t const m = 0x0000080e70100000UL;      // Some input value (doesn't have to be a const).

  // The lambda is marked mutable because of the asm() that claims to change m,
  // however - you should not *really* change the input variables!
  // The [m = m] is needed because our m is const and doing just [m] weirdly
  // enough makes the type of the captured m also const, despite the mutable.
  auto result = stopwatch.measure<nk>(loopsize, [m = m]() mutable {
      uint64_t lsb;                     // Declare any (temporary) "output" variables.
      //IACA_START                      // Optional; needed when you want to analyse the generated assembly code with IACA
                                        // (https://software.intel.com/en-us/articles/intel-architecture-code-analyzer).
      asm volatile ("" : "+r" (m));     // See https://stackoverflow.com/a/54245040/1487069 for an explanation and discussion.

      // Code under test.
      lsb = m & -m;                     // This will result in three assembly instructions.

      asm volatile ("" :: "r" (lsb));   // Same.
      //IACA_END                        // Optional; needed when you want to analyse the generated assembly code with IACA.
  }, minimum_of);

  std::cout << "Result: " << (result / cpu_frequency * 1e9 / loopsize) << " ns [measured " << result << " clocks]." << std::endl;
}

#endif // EXAMPLE_CODE

namespace benchmark {

unsigned int constexpr cache_line_size = 64;    // grep cache_alignment /proc/cpuinfo
unsigned int constexpr number_of_cpus = 32;     // See /proc/cpuinfo

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

  unsigned int calibrated_iterations;   // The iterations value last passed to calibrate_overhead().
  uint32_t iterations_overhead;         // The overhead when using calibrated_iterations, in clock cycles.

 public:
  static int s_stopwatch_overhead;      // The overhead of calling start()/stop(), in clock cycles.

 public:
  static constexpr unsigned int cpu_any = 0xffffffff;  // This value means: keep running on whatever cpu this thread is running.

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
  [[gnu::always_inline]] void start()
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
  [[gnu::always_inline]] void stop()
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

  // Measure the number of clock cycles that it takes to run functor() iterations times
  // and return to smallest value of doing that minimum_of times.
  template<class T>
  uint64_t get_minimum_of(unsigned int const iterations, T const functor, unsigned int const minimum_of)
  {
    uint64_t cycles = std::numeric_limits<uint64_t>::max();
    T benchmark_code = functor;
    for (unsigned int i = 0; i < minimum_of; ++i)
    {
      start();
      for (unsigned int j = 0; j < iterations; ++j)
        benchmark_code();
      stop();
      uint64_t ncycles = diff_cycles();
      if (ncycles < cycles)
        cycles = ncycles;
    }
    return cycles;
  }

  // Same as above but correct for loop and stopwatch overhead (call calibrate_overhead() first!),
  // as well as repeat calling get_minimum_of() until we are 99.9% sure what measurement occurs
  // most often (to get something that will reproduce extremely well).
  template<int nk = 3, class T>
  eda::FrequencyCounterResult measure(unsigned int iterations, T const functor, unsigned int minimum_of = 3)
  {
    eda::FrequencyCounter<int, nk> fc;
    while (!fc.add(get_minimum_of(iterations, functor, minimum_of)))
      ;
    eda::FrequencyCounterResult result = fc.result();
    Dout(dc::notice, "Measured with overhead: " << result.m_cycles);
    result.m_cycles -= s_stopwatch_overhead;
    if (iterations == calibrated_iterations)
      result.m_cycles -= iterations_overhead;
    if (result.m_cycles < 0)
      result.m_cycles = 0;
    return result;
  }

  void calibrate_overhead(size_t iterations, size_t minimum_of);

  friend std::ostream& operator<<(std::ostream& os, Stopwatch const& stopwatch);
};

} // namespace benchmark
