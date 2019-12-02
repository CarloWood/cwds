/**
 * @file
 * @brief This file contains the declarations of namespace eda.
 *
 * @Copyright (C) 2018  Carlo Wood.
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

#include <map>
#include <array>
#include <cstdlib>
#include <iostream>
#include "debug.h"

namespace eda {

struct FrequencyCounterResult
{
  int m_cycles;

  enum type_nt { t999, tm1, tm2 } m_type;

  operator int() const { return m_cycles; }
  bool is_t999() const { return m_type == t999; }
  bool is_tm1() const { return m_type == tm1; }
  bool is_tm2() const { return m_type == tm2; }
};

// Count the number of occurrances of type T, added to this object with FrequencyCounter<T>::add(T).
// Keeps track of the nk most frequently occurring values of T.
template<typename T, int nk = 1>
class FrequencyCounter
{
  struct Data { size_t count; int k; Data(size_t count_, int k_) : count(count_), k(k_) { } };
  using counters_type = std::map<T, Data>;
  using iterator = typename counters_type::iterator;

  counters_type m_counters;
  iterator m_hint;
  std::array<iterator, nk> m_max_iters;
  FrequencyCounterResult m_result;

 public:
  FrequencyCounter() : m_hint(m_counters.end())
  {
    for (int i = 0; i < nk; ++i)
      m_max_iters[i] = m_counters.end();
  }

  bool add(T value);    // Returns true when result() became valid.
  T most() const { return m_max_iters[0]->first; }
  FrequencyCounterResult result() const { return m_result; }
  double average() const;

  counters_type const& counters() const { return m_counters; }

  void print_on(std::ostream& os) const;
};

template<typename T, int nk>
void FrequencyCounter<T, nk>::print_on(std::ostream& os) const
{
  os << "map:" << std::endl;
  for (auto&& counter : m_counters)
  {
    os << "value: " << counter.first << "; data = { count = " << counter.second.count << ",  k = " << counter.second.k << "}" << std::endl;
  }
  os << "max count/value/iter";
  for (int i = 0; i < nk; ++i)
  {
    if (m_max_iters[i] != m_counters.end())
    {
      os << " {" << m_max_iters[i]->first << ", " << m_max_iters[i]->second.count << '}';
      ASSERT(m_max_iters[i]->second.k == i);
    }
    else
      os << " <empty>";
  }
  os << std::endl;
}

template<typename T, int nk>
bool FrequencyCounter<T, nk>::add(T value)
{
  m_hint = m_counters.emplace_hint(m_hint, value, Data{0, -1});
  size_t count = ++(m_hint->second.count);
  int k = m_hint->second.k;
  if (k == -1)
  {
    k = nk - 1;
    if (m_max_iters[k] == m_counters.end() || count > m_max_iters[k]->second.count)
    {
      while (k > 0 && (m_max_iters[k - 1] == m_counters.end() || m_max_iters[k - 1]->second.count == count - 1))
        --k;
      if (m_max_iters[k] != m_counters.end())
        m_max_iters[k]->second.k = -1;
      m_max_iters[k] = m_hint;
      m_hint->second.k = k;
    }
  }
  else if (nk > 1)
  {
    int new_k = k;
    while (new_k > 0 && m_max_iters[new_k - 1]->second.count == count - 1)
      --new_k;
    if (new_k != k)
    {
      std::swap(m_max_iters[k], m_max_iters[new_k]);
      m_max_iters[k]->second.k = k;
      m_max_iters[new_k]->second.k = new_k;
    }
  }
  int m2;
  for (int i = 0; i < nk - 1 && m_max_iters[i + 1] != m_counters.end() && (m2 = m_max_iters[i + 1]->second.count) > 10; ++i)
  {
    int m1 = m_max_iters[i]->second.count;
    ASSERT(m1 >= m2);
    double test_statistic = 1.0 * (m1 - m2) * (m1 - m2) / (m1 + m2);
    if (test_statistic > 10.828)
    {
      m_result.m_cycles = m_max_iters[0]->first;
      m_result.m_type = (i == 0) ? FrequencyCounterResult::t999 : (i == 1) ? FrequencyCounterResult::tm1 : FrequencyCounterResult::tm2;
      size_t sum = 0;
      for (int j = 0; j <= i; ++j)
        sum += m_max_iters[j]->first;
      m_result.m_cycles = sum / (i + 1);
      return true;
    }
    if (std::abs(typename std::make_signed<T>::type(m_max_iters[i]->first - m_max_iters[i - 1]->first)) > 1)
      break;
  }
  return false;
}

template<typename T, int nk>
double FrequencyCounter<T, nk>::average() const
{
  double avg = 0;
  size_t count = 0;
  for (int i = 0; i < nk && m_max_iters[i] != m_counters.end(); ++i)
  {
    avg += (double)m_max_iters[i]->first * m_max_iters[i]->second.count;
    count += m_max_iters[i]->second.count;
  }
  return avg / count;
}

} // namespace eda
