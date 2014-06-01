/*
 *  tutte.hpp
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

#ifndef TUTTE_HPP
#define TUTTE_HPP

#include "connectivity/connectivity.hpp"
#include <boost/unordered/unordered_map.hpp>

template<class Weight>
class tutte
{
  const Weight Q;
  const Weight v;

public:
  using weight_type = Weight ;
  using table_type = boost::unordered_map<connectivity, weight_type>;

  template<class T, class U>
  tutte(T const& Q_, U const& v_) : Q(Q_), v(v_) {}

  table_type empty_state(unsigned int size) const
  {
    table_type tmp_table;
    tmp_table[connectivity(size)] = Weight(1);
    return tmp_table;
  }

  table_type
  join_operator(unsigned int i, unsigned int j, table_type const& t) const
  {
    table_type tmp_table;
    for (auto const & e : t) {
      tmp_table[e.first] += e.second;
      tmp_table[connectivity(e.first).connect(i, j).canonicalize()]
        += e.second * v;
    }
    return tmp_table;
  }

  table_type
  delete_operator(unsigned int i, table_type const& t) const
  {
    table_type tmp_table;
    for (auto const& e : t) {
      tmp_table[connectivity(e.first).delete_node(i).canonicalize()]
        += (e.first.singleton(i) ? (e.second * Q) : e.second);
    }
    return tmp_table;
  }

  template<class Mapping>
  table_type
  table_fusion(Mapping A_to_B,
    table_type const& A_table,
    table_type const& B_table) const
  {
    table_type tmp_table;
    for (auto const & eA : A_table) {
      for (auto const & eB : B_table) {
        connectivity c = eB.first;
        eA.first.decompose([&](unsigned int i, unsigned int j) {
          c.connect(A_to_B[i], A_to_B[j]);
        });
        tmp_table[c.canonicalize()] += eA.second * eB.second;
      }
    }
    return tmp_table;
  }
};
#endif
