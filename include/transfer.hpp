/*
 *  transfer.hpp
 *
 *
 *  Created by Andrea Bedinion 13/Jul/2009.
 *  Copyright 2009, 2010, 2011 Andrea Bedini.
 *
 *  Distributed under the terms of the GNU General Public License.
 *  The full license is in the file COPYING, distributed as part of
 *  this software.
 *
 */

#ifndef TRANSFER_HPP
#define TRANSFER_HPP

#include "tree_decomposition/tree_decomposition.hpp"

#include <boost/graph/graph_traits.hpp>
#include <boost/range/algorithm/set_algorithm.hpp>

namespace transfer {
  // Tree decomposition typedefs
  typedef tree_decomposition::tree_decomposition tree_type;
  typedef tree_decomposition::bag                bag_type;

  template<class Operators>
  typename Operators::table_type
  recurse(const Operators& op, const tree_type& t, tree_type::iterator ti)
  {
    // give an easy name to this bag
    bag_type const& b(*ti);

    // create a new table containing only the empty state
    const unsigned int n = ti->vertices.size();
    typename Operators::table_type table = op.empty_state(n);

    // iterates over children
    tree_type::sibling_iterator sib, sib_end;
    for (sib = t.begin(ti), sib_end = t.end(ti); sib != sib_end; ++sib) {
      // recurse
      typename Operators::table_type table_sib = recurse(op, t, sib);

      // make a copy of this bag since we don't want to touch the tree
      // decomposition
      bag_type b_sib(*sib);

      // diffe contains the vertices in b_sib which are not in b (the parent bag)
      std::vector<unsigned int> diffe;
      boost::set_difference(b_sib.vertices, b.vertices, std::back_inserter(diffe));

      // delete each vertex not present in the parent bag
      BOOST_FOREACH(unsigned int v, diffe) {
	table_sib = op.delete_operator(b_sib.vertices.index(v), table_sib);
	b_sib.vertices.remove(v);
      }

      // create b_sib to b bag mapping
      const unsigned int A_size = b_sib.vertices.size();
      std::vector<unsigned int> A_to_B(A_size);
      for (unsigned int i = 0; i < A_size; ++i)
	A_to_B[i] = b.vertices.index(b_sib.vertices.at(i));

      table = op.table_fusion(A_to_B, table_sib, table);
    }

    // apply the join operator for each edge in the bag
    std::pair<unsigned int, unsigned int> e;
    BOOST_FOREACH(e, b.edges) {
      table = op.join_operator(b.vertices.index(e.first),
			       b.vertices.index(e.second), table);
    }
    return table;
  }

  template<class Operators>
  typename Operators::weight_type
  transfer(const Operators& op, const tree_type& t)
  {
    tree_type::iterator ti = t.begin();
    typename Operators::table_type table = recurse(op, t, ti);

    // make a copy of this bag since we don't want to touch the tree
    // decomposition
    bag_type b(*ti);

    BOOST_FOREACH(unsigned int v, ti->vertices) {
      table = op.delete_operator(b.vertices.index(v), table);
      b.vertices.remove(v);
    }
    assert(table.size() == 1);
    return table.begin()->second;
  }
}

#endif
