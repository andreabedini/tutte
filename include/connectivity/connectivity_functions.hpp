/*
 *  connectivity_functions.hpp
 *
 *
 *  Created by Andrea Bedini on 24/Nov/2011.
 *  Copyright (c) 2011-2014, Andrea Bedini <andrea.bedini@gmail.com>.
 *
 *  Distributed under the terms of the Modified BSD License.
 *  The full license is in the file COPYING, distributed as part of
 *  this software.
 *
 */

#ifndef CONNECTIVITY_FUNCTIONS_HPP
#define CONNECTIVITY_FUNCTIONS_HPP

#include <boost/cstdint.hpp>

#include <algorithm>
#include <cstddef>

namespace connectivity_functions {
  using boost::uint8_t;

  void connect(unsigned int n, uint8_t* a, unsigned int i, unsigned int j)
  {
    uint8_t li = a[i], lj = a[j];
    std::replace(a, a + n, lj, li);
  }

  void detach(unsigned int n, uint8_t* a, unsigned int i)
  {
    a[i] = n + 1;
  }

  bool connected(const uint8_t* a, unsigned int i, unsigned int j)
  {
    return a[i] == a[j];
  }

  bool is_singleton(unsigned int n, const uint8_t* a, unsigned int i)
  {
    return std::count(a, a + n, a[i]) == 1;
  }

  void canonicalize(unsigned int n, uint8_t* a)
  {
    const unsigned int max = 50;
    uint8_t table[max];
    std::fill(table, table + max, 0);

    unsigned int k = 1;
    for (unsigned int i = 0; i < n; ++i) {
      if (table[a[i]])
	a[i] = table[a[i]];
      else {
	table[a[i]] = k;
	a[i] = k;
	++ k;
      }
    }
  }

  template<class F>
  void decompose(unsigned int n, const uint8_t* a, F f)
  {
    const unsigned int max = 50;
    int table[max];
    std::fill(table, table + max, -1);

    unsigned int j = 0;
    for (unsigned int i = 0; i < n; ++i) {
      if (table[a[i]] != -1)
	f(table[a[i]], j);
      table[a[i]] = j;
      j ++;
    }
  }
}

#endif
