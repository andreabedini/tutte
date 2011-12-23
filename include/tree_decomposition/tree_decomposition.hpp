/*
 *  tree_decomposition.hpp
 *
 *
 *  Created by Andrea Bedini on 14/Jul/09.
 *  Copyright 2009, 2010, 2011 Andrea Bedini. 
 *
 *  Distributed under the terms of the GNU General Public License.
 *  The full license is in the file COPYING, distributed as part of
 *  this software.
 *
 */

#ifndef TREE_DECOMPOSITION_HPP
#define TREE_DECOMPOSITION_HPP

#include "tree_decomposition/tree.hh"
#include "utility/smallset.hpp"

#include <boost/foreach.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/range/algorithm/find_first_of.hpp>
#include <boost/property_map/vector_property_map.hpp>

#include <algorithm>
#include <iosfwd>
#include <vector>

namespace tree_decomposition {
  struct bag {
    smallset<unsigned int> vertices;
    std::vector<std::pair<unsigned int, unsigned int> > edges;
  };

  typedef tree<bag> tree_decomposition;

  template<class Graph, class Range>
  tree_decomposition
  build_tree_decomposition(Range const& order, Graph g)
  // please note that g is passed by copy
  {
    using namespace std;
    using namespace boost;

    typedef typename Graph::vertex_descriptor vertex_descriptor;
    typedef typename Graph::edge_descriptor   edge_descriptor;

    vector<vertex_descriptor> vertices(num_vertices(g));

    // list (stable) vertex descriptors in the user-supplied order
    unsigned int i = 0;
    BOOST_FOREACH(unsigned int vi, order)
      vertices[i++] = vertex(vi, g);

    // bags will be indexed by vertex indices
    vector<bag> bags(num_vertices(g));

    // each bag's parent index need also to be stored
    vector<unsigned int> parent(num_vertices(g));

    // we need this map to differenciate between edges added during
    // vertex eliminations from the ones originally present in the
    // graph
    vector_property_map
      < bool
      , typename property_map<Graph, edge_index_t>::type
      > filling_edge(num_edges(g), get(edge_index, g));

    // iterate over these vertices in order
    BOOST_FOREACH(vertex_descriptor v, vertices) {
      // this is the index
      unsigned int vi = get(vertex_index, g, v);

      // add to each vertex bag the vertex (index) vi
      bags[vi].vertices.insert(vi);
      BOOST_FOREACH(edge_descriptor e, out_edges(v, g)) {
	// and add its neighbours
	unsigned int ui = get(vertex_index, g, target(e, g));
	bags[vi].vertices.insert(ui);
	// add the edge to the bag if the edge is 'genuine'
	if (not filling_edge[e])
	  bags[vi].edges.push_back(make_pair(vi, ui));
      }

      // if we find the parent vertex index
      if (out_degree(v, g) > 0) {
	vertex_descriptor u = *find_first_of(vertices, adjacent_vertices(v, g));
	parent[vi] = get(vertex_index, g, u);
      }

      // eliminate vertex, marking filling edges as we add them
      BOOST_FOREACH(vertex_descriptor a, adjacent_vertices(v, g)) {
	BOOST_FOREACH(vertex_descriptor b, adjacent_vertices(v, g)) {
	  if (a != b and not edge(a, b, g).second) {
	    edge_descriptor e = add_edge(a, b, num_edges(g), g).first;
	    filling_edge[e] = true;
	  }
	}
      }
      clear_vertex(v, g);
      remove_vertex(v, g);
    }

    // ok, now we have a list of bags each with a parent index
    // time to unroll the tree
    tree_decomposition tree;

    // again we need a vector of tree nodes (booring!)
    vector<tree_decomposition::iterator> nodes(bags.size());

    // iterate the vertex ordering backward, the first (= last) bag
    // is going to be the tree root
    typename Range::const_reverse_iterator ri = order.rbegin(), ri_end = order.rend();
    nodes[*ri] = tree.set_head(bags[*ri]);
    ++ ri;
    while (ri != ri_end) {
      nodes[*ri] = tree.append_child(nodes[parent[*ri]], bags[*ri]);
      ++ ri;
    }

    return tree;
  }

  unsigned int tree_width(const tree_decomposition& t)
  {
    unsigned int max = 0;
    tree_decomposition::iterator ti;
    for(ti = t.begin(); ti != t.end(); ++ti)
      if (ti->vertices.size() > max)
	max = ti->vertices.size();
    return max - 1;
  }

  void print_tree(std::ostream& o,
		  tree_decomposition::sibling_iterator ti,
		  const tree_decomposition& t)
  {
    o << "( ";
    BOOST_FOREACH(unsigned int v, ti->vertices) {
      o << v << " ";
    }
    if (not ti->edges.empty()) {
      o << "| ";
      std::pair<unsigned, unsigned> p;
      BOOST_FOREACH(p, ti->edges) {
	o << p.first << "-" << p.second << " ";
      }
    }
    o << ") ";
    tree_decomposition::sibling_iterator
      sib = t.begin(ti), sib_end = t.end(ti);
    if (sib != sib_end) {
      o << "{ ";
      for (; sib != sib_end; ++sib)
	print_tree(o, sib, t);
      o << "} ";
    }
  }

  std::ostream& operator<<(std::ostream& o, const tree_decomposition& t)
  {
    print_tree(o, t.begin(), t);
    return o;
  }
}

#endif
