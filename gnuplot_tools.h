/**
 * @file
 * @brief This file contains the declarations of namespace eda.
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

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <limits>
#include <cstddef>
#include <iostream>
#include "gnuplot-iostream/gnuplot-iostream.h"  // Please also install https://github.com/dstahlke/gnuplot-iostream.git
#include "FrequencyCounter.h"
#include "debug.h"

// https://en.wikipedia.org/wiki/Exploratory_Data_Analysis
namespace eda {

template<typename T>
class MinAvgMax
{
 private:
  T m_min;
  T m_max;
  T m_sum;
  size_t m_cnt;

 public:
  T min() const { return m_min; }
  T max() const { return m_max; }
  T avg() const { return m_sum / m_cnt; }

  MinAvgMax() : m_min(std::numeric_limits<T>::max()), m_max(-std::numeric_limits<T>::max()), m_sum(0.0), m_cnt(0) { }

  void data_point(T data)
  {
    if (data < m_min)
      m_min = data;
    if (data >m_max)
      m_max = data;
    m_sum += data;
    ++m_cnt;
  }

  size_t count() const { return m_cnt; }

  friend std::ostream& operator<<(std::ostream& os, MinAvgMax<T> const& mma)
  {
    os << mma.m_min << '/' << mma.avg() << '/' << mma.m_max;
    return os;
  }
};

class Plot
{
 protected:
  Gnuplot gp;
  std::string m_title;
  std::string m_xlabel;
  std::string m_ylabel;
  std::string m_header;
  std::mutex m_mutex;
  std::map<std::string, std::vector<boost::tuple<double, double, double>>> m_map;
  std::vector<std::string> m_cmds;
  double m_x_min;
  double m_x_max;
  double m_y_min;
  double m_y_max;

 public:
  Plot(std::string title, std::string xlabel, std::string ylabel) :
    m_title(title), m_xlabel(xlabel), m_ylabel(ylabel), m_x_min(0.0), m_x_max(0.0), m_y_min(0.0), m_y_max(0.0) { }

  void set_xrange(double x_min, double x_max) { m_x_min = x_min; m_x_max = x_max; }
  void set_yrange(double y_min, double y_max) { m_y_min = y_min; m_y_max = y_max; }
  void set_header(std::string header) { m_header = header; }
  void add_data_point(double x, double y, double dy, std::string const& description)
  {
    std::unique_lock<std::mutex> lk(m_mutex);
    m_map[description].emplace_back(x, y, dy);
  }

  bool has_data() const { return !m_map.empty(); }
  void add(std::string const& str) { m_cmds.push_back(str); }
  size_t points(std::string key) const { auto iter = m_map.find(key); return iter == m_map.end() ? 0 : iter->second.size(); }

  void show(std::string with = "")
  {
    Dout(dc::notice|flush_cf|continued_cf, "Generating graph... ");
    gp << "set title '" << m_title << "' font \"helvetica,12\"\n";
    gp << "set xlabel '" << m_xlabel << "'\n";
    gp << "set ylabel '" << m_ylabel << "'\n";
    if (m_x_max > 0.0)
      gp << "set xrange [" << m_x_min << ":" << m_x_max << "]\n";
    else
      gp << "set xrange [" << m_x_min << ":]\n";
    if (m_y_max > 0.0)
      gp << "set yrange [" << m_y_min << ":" << m_y_max << "]\n";
    else
      gp << "set yrange [" << m_y_min << ":]\n";
    for (auto&& s : m_cmds)
      gp << s << '\n';
    char const* separator = "plot ";
    for (auto&& e : m_map)
    {
      gp << separator << "'-'";
      if (!m_header.empty())
        gp << ' ' << m_header;
      if (!with.empty())
        gp << " with " << with;
      gp << " title '" << e.first << "'";
      separator = ", ";
    }
    gp << '\n';
    for (auto&& e : m_map)
      gp.send1d(e.second);
    Dout(dc::finish|flush_cf, "done");
  }
};

class PlotHistogram : public Plot
{
 private:
  double m_bucket_width;
  MinAvgMax<double> m_mam;

 public:
  PlotHistogram(std::string title, std::string xlabel, std::string ylabel, int bucket_width = 1) : Plot(title, xlabel, ylabel), m_bucket_width(bucket_width) { }

  template<typename T>
  void show(FrequencyCounter<T> const& frequence_counter, char const* key = "data")
  {
    auto const& counters = frequence_counter.counters();
    for (auto&& e : counters)
      add_data_point(e.first, e.second, key);
    show();
  }

  void add_data_point(double value, int count, std::string const& description)
  {
    m_mam.data_point(value);
    Plot::add_data_point(value, count, 0, description);
  }

  void show()
  {
    if (m_x_min == 0.0 && m_x_max == 0.0)
      set_xrange(m_mam.min() - m_bucket_width, m_mam.max() + m_bucket_width);
    set_header("smooth freq");
    add("set boxwidth " + std::to_string(m_bucket_width));
    add("set style fill solid 0.5");
    //add("set xtics " + std::to_string(5 * m_bucket_width) + " rotate");
    //add("set mxtics 5");
    add("set tics out");
    add("unset key");
    Plot::show("boxes");
  }
};

} // namespace eda
