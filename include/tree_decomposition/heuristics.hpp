/*
 *  heuristics.hpp
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

#ifndef HEURISTICS_HPP
#define HEURISTICS_HPP

#include "tree_decomposition/eliminate_vertex.hpp"
#include "tree_decomposition/tree_decomposition.hpp"

#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/graph/graph_traits.hpp>

#include <algorithm>

namespace {
  template<class Graph>
  struct degree_compare {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    const Graph &g;

    degree_compare(const Graph& g) : g(g) {}
    bool operator()(vertex_descriptor v, vertex_descriptor u) const {
      return degree(v, g) < degree(u, g);
    }
  };


  template<class Graph>
  class fillin_compare {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<Graph>::adjacency_iterator adjacency_iterator;

    const Graph &g;

    unsigned int num_non_adjacent_neighbors(vertex_descriptor v) const
    {
      unsigned int n = 0;
      BOOST_FOREACH(vertex_descriptor u, adjacent_vertices(v, g)) {
	adjacency_iterator ai_u, ai_end_u;
	tie(ai_u, ai_end_u) = adjacent_vertices(u, g);
	BOOST_FOREACH(vertex_descriptor z, adjacent_vertices(v, g)) {
	  if (std::find(ai_u, ai_end_u, z) != ai_end_u)
	    n ++;
	}
      }
      return n;
    }
  public:
    fillin_compare(const Graph& g) : g(g) {}

    bool operator()(vertex_descriptor v, vertex_descriptor u) const {
      return num_non_adjacent_neighbors(v) < num_non_adjacent_neighbors(u);
    }
  };
}

namespace heuristics {
  using namespace boost;
  using namespace boost::adaptors;

  template<class Graph, class OutputIterator>
  void greedy_degree_order(Graph g, OutputIterator out) {
    typedef typename Graph::vertex_descriptor vertex_descriptor;

    degree_compare<Graph> compare(g);
    while (num_vertices(g) > 0) {
      vertex_descriptor v = *min_element(vertices(g), compare);
      *out++ = get(vertex_index, g, v);
      eliminate_vertex(v, g);
    }
  }

  //////////////////////////////////////////////////////////////////////

  template<class Graph, class OutputIterator>
  void greedy_fillin_order(Graph g, OutputIterator out) {
    typedef typename Graph::vertex_descriptor vertex_descriptor;

    fillin_compare<Graph> compare(g);
    while (num_vertices(g) > 0) {
      vertex_descriptor v = *min_element(vertices(g), compare);
      *out++ = get(vertex_index, g, v);
      eliminate_vertex(v, g);
    }
  }

  //////////////////////////////////////////////////////////////////////

  template<class Graph, class OutputIterator>
  void greedy_local_degree_order(Graph g, OutputIterator out) {
    typedef typename Graph::vertex_descriptor vertex_descriptor;
    
    degree_compare<Graph> compare(g);
    vertex_descriptor current = *min_element(vertices(g), compare);
    while (num_vertices(g) > 0) {
      *out++ = get(vertex_index, g, current);
      vertex_descriptor next = *min_element(adjacent_vertices(current, g), compare);
      eliminate_vertex(current, g);
      current = next;
    }
  }

  //////////////////////////////////////////////////////////////////////

  template<class Graph, class OutputIterator>
  void greedy_local_fillin_order(Graph g, OutputIterator out) {
    typedef typename Graph::vertex_descriptor vertex_descriptor;

    fillin_compare<Graph> compare(g);
    vertex_descriptor current = *min_element(vertices(g), compare);
    while (num_vertices(g) > 0) {
      *out++ = get(vertex_index, g, current);
      vertex_descriptor next = *min_element(adjacent_vertices(current, g), compare);
      eliminate_vertex(current, g);
      current = next;
    }
  }
}

#endif
