/*
 *  tutte.hpp
 *
 *
 *  Created by Andrea Bedini on 24/Nov/2011.
 *  Copyright 2011 Andrea Bedini.
 *
 *  Distributed under the terms of the GNU General Public License.
 *  The full license is in the file COPYING, distributed as part of
 *  this software.
 *
 */

#ifndef TUTTE_HPP
#define TUTTE_HPP

#include "transfer.hpp"
#include "connectivity/connectivity.hpp"
#include <boost/unordered/unordered_map.hpp>

namespace {
  template<class M, class C>
  class mapped_connect {
    M m;
    C& c;
  public:
    mapped_connect(M m, C& c) : m(m), c(c)
    { }
    
    void operator()(unsigned int i, unsigned int j) const {
      c.connect(m[i], m[j]);
    }
  };
    
  template<class M, class C>
  mapped_connect<M, C> make_mapped_connect(M m, C& c) {
    return mapped_connect<M, C>(m, c);
  }
}

template<class Weight>
class tutte
{
  const Weight Q;
  const Weight v;

public: 
  typedef Weight weight_type;
  typedef boost::unordered_map<connectivity, weight_type> table_type;
  typedef typename table_type::const_iterator table_const_iterator;

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
    for (table_const_iterator it = t.begin(); it != t.end(); ++it) {
      tmp_table[it->first] += it->second;
      tmp_table[connectivity(it->first).connect(i, j).canonicalize()]
	+= it->second * v;
    }
    return tmp_table;
  }
    
  table_type
  delete_operator(unsigned int i, table_type const& t) const
  {
    table_type tmp_table;
    for (table_const_iterator it = t.begin(); it != t.end(); ++it) {
      tmp_table[connectivity(it->first).delete_node(i).canonicalize()]
	+= it->first.singleton(i) ? (it->second * Q) : it->second;
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
    table_const_iterator iA, iB;
    for (iA = A_table.begin(); iA != A_table.end(); ++ iA) {
      for (iB = B_table.begin(); iB != B_table.end(); ++ iB) {
	connectivity c = iB->first;
	iA->first.decompose(make_mapped_connect(A_to_B, c));
	tmp_table[c.canonicalize()] += iA->second * iB->second;
      }
    }
    return tmp_table;
  }
};

#endif
