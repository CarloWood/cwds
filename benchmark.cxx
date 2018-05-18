/**
 * @file
 * @brief This file contains the definitions of namespace benchmark.
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

#include "sys.h"
#include "benchmark.h"
#include "debug.h"
#include <cstring>
#include <pthread.h>
#include <sys/syscall.h>
#include <stdexcept>
#include <system_error>

extern "C" long syscall(long number, ...);

namespace benchmark {

void* Stopwatch::operator new(size_t size)
{
  void* ptr;
  // Use storage that is aligned with a cache line.
  if (int err_num = posix_memalign(&ptr, cache_line_size, size))
  {
    if (err_num == ENOMEM)      // There was insufficient memory to fulfill the allocation request.
      throw std::bad_alloc{};
    // else EINVAL              // The alignment argument was not a power of two, or was not a multiple of sizeof(void *).
    throw std::invalid_argument("posix_memalign: cache_line_size is not a power of two or is not a multiple of sizeof(void*)");
  }
  return ptr;
}

Stopwatch::Stopwatch(unsigned int cpu_nr)
{
  char const* err_str;
  int err_num;

  std::memset(this, 0, sizeof(Stopwatch));
  do
  {
    // Pin main thread to a cpu 'cpu_nr'.
    pthread_t thread = pthread_self();

    // If cpu_nr is cpu_any try to pin the thread on the CPU that it is currently running on.
    if (cpu_nr == cpu_any)
    {
      unsigned cpu;
      unsigned node;

      if (syscall(SYS_getcpu, &cpu, &node, nullptr) == -1)
      {
        err_str = "getcpu()";
        err_num = errno;
        break;
      }
      cpu_nr = cpu;
    }

    size_t const cpu_set_size = CPU_ALLOC_SIZE(number_of_cpus);
    m_cpuset = CPU_ALLOC(number_of_cpus);
    ASSERT(m_cpuset != nullptr);
    CPU_ZERO_S(cpu_set_size, m_cpuset);
    CPU_SET_S(cpu_nr, cpu_set_size, m_cpuset);
    err_num = pthread_setaffinity_np(thread, cpu_set_size, m_cpuset);
    if (err_num)
    {
      err_str = "pthread_setaffinity_np()";
      CPU_FREE(m_cpuset);
      m_cpuset = nullptr;
    }
#ifdef CWDEBUG
    else
    {
      // Make sure we're now successfully running on a single CPU.
      err_num = pthread_getaffinity_np(thread, cpu_set_size, m_cpuset);
      if (err_num)
        err_str = "pthread_getaffinity_np()";
      else
      {
        Dout(dc::notice|continued_cf, "Stopwatch at " << (void*)this << ", thread " << (void*)thread << ", is restricted to CPU");
	for (unsigned int j = 0; j < number_of_cpus; ++j)
	  if (CPU_ISSET_S(j, cpu_set_size, m_cpuset))
            Dout(dc::continued, ' ' << j);
        Dout(dc::finish, '.');
        ASSERT(CPU_COUNT_S(cpu_set_size, m_cpuset) == 1);
      }
    }
#endif
    if (err_num)
      break;

    // Success.
    prefetch();
    return;
  }
  while (0);

  // Report error.
  throw std::system_error(err_num, std::generic_category(), err_str);
}

Stopwatch::~Stopwatch()
{
  if (m_cpuset)
  {
    // Restore CPU affinity.
    size_t const cpu_set_size = CPU_ALLOC_SIZE(number_of_cpus);
    DEBUG_ONLY(int err_num =) pthread_setaffinity_np(pthread_self(), cpu_set_size, m_cpuset);
    Dout(dc::warning(err_num), "Failed to restore cpu affinity.");
    CPU_FREE(m_cpuset);
  }
}

} // namespace benchmark
