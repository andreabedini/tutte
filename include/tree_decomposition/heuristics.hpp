/*
 *  heuristics.hpp
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

#ifndef HEURISTICS_HPP
#define HEURISTICS_HPP

#include <boost/range/algorithm.hpp>

namespace heuristics {
  using namespace boost;

  template<class Vertex, class Graph>
  void eliminate_vertex(Vertex v, Graph& g) {
    for (auto a : as_range(adjacent_vertices(v, g))) {
      for (auto b : as_range(adjacent_vertices(v, g))) {
        if (a != b and not edge(a, b, g).second) {
          add_edge(a, b, g);
        }
      }
    }
    clear_vertex(v, g);
    remove_vertex(v, g);
  }

  template<class Graph, class Vertex>
  unsigned int num_non_adjacent_neighbors(Vertex v, Graph const& g)
  {
    unsigned int n = 0;
    for (auto u : as_range(adjacent_vertices(v, g))) {
      auto adj = adjacent_vertices(u, g);
      for (auto z : as_range(adjacent_vertices(v, g))) {
        if (boost::find(adj, z) != end(adj))
          n ++;
      }
    }
    return n;
  }

  template<class Graph, class OutputIterator>
  void greedy_degree_order(Graph g, OutputIterator out) {
    using vertex = typename Graph::vertex_descriptor;
    while (num_vertices(g) > 0) {
      auto v = *min_element(vertices(g), [&](vertex v, vertex u) {
        return degree(v, g) < degree(u, g);
      });
      *out++ = get(vertex_index, g, v);
      eliminate_vertex(v, g);
    }
  }

  //////////////////////////////////////////////////////////////////////

  template<class Graph, class OutputIterator>
  void greedy_fillin_order(Graph g, OutputIterator out) {
    using vertex = typename Graph::vertex_descriptor;
    while (num_vertices(g) > 0) {
      auto v = *min_element(vertices(g), [&](vertex v, vertex u) {
        return num_non_adjacent_neighbors(v, g) < num_non_adjacent_neighbors(u, g);
      });
      *out++ = get(vertex_index, g, v);
      eliminate_vertex(v, g);
    }
  }

  //////////////////////////////////////////////////////////////////////

  template<class Graph, class OutputIterator>
  void greedy_local_degree_order(Graph g, OutputIterator out) {
    using vertex = typename Graph::vertex_descriptor;
    auto compare = [&](vertex v, vertex u) {
      return degree(v, g) < degree(u, g);
    };
    vertex current = *min_element(vertices(g), compare);
    while (num_vertices(g) > 0) {
      *out++ = get(vertex_index, g, current);
      vertex next = *min_element(vertices(g), compare);
      eliminate_vertex(current, g);
      current = next;
    }
  }

  //////////////////////////////////////////////////////////////////////

  template<class Graph, class OutputIterator>
  void greedy_local_fillin_order(Graph g, OutputIterator out) {
    using vertex = typename Graph::vertex_descriptor;
    auto compare =  [&](vertex v, vertex u) {
      return num_non_adjacent_neighbors(v, g) < num_non_adjacent_neighbors(u, g);
    };
    vertex current = *min_element(vertices(g), compare);
    while (num_vertices(g) > 0) {
      *out++ = get(vertex_index, g, current);
      vertex next = *min_element(adjacent_vertices(current, g), compare);
      eliminate_vertex(current, g);
      current = next;
    }
  }
}

#endif
