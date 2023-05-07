#include "cwds/benchmark.h"

int main()
{
  unsigned int const cpu = 0;
  size_t const loopsize = 1000;
  size_t const minimum_of = 3;
  int const nk = 3;

  benchmark::Stopwatch stopwatch(cpu);
  stopwatch.calibrate_overhead(loopsize, minimum_of);
  uint64_t const m = 0x0000080e70100000UL;
  auto result = stopwatch.measure<nk>(loopsize, [m = m]() mutable {
      uint64_t lsb;
      asm volatile ("" : "+r" (m));
      lsb = m & -m;
      asm volatile ("" :: "r" (lsb));
  }, minimum_of);
}
