/*
 *  tree_decomposition.hpp
 *
 *
 *  Created by Andrea Bedini on 14/Jul/09.
 *  Copyright (c) 2009-2014, Andrea Bedini <andrea.bedini@gmail.com>. 
 *
 *  Distributed under the terms of the Modified BSD License.
 *  The full license is in the file COPYING, distributed as part of
 *  this software.
 *
 */

#ifndef TREE_DECOMPOSITION_HPP
#define TREE_DECOMPOSITION_HPP

#include "utility/smallset.hpp"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/find_first_of.hpp>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <iosfwd>
#include <vector>

namespace {
  // this is because pair range access removed from C++11
  // http://stackoverflow.com/questions/6167598/why-was-pair-range-access-removed-from-c11
  template<class Iter>
  struct iter_pair_range : std::pair<Iter,Iter> {
    iter_pair_range(std::pair<Iter,Iter> const& x)
    : std::pair<Iter,Iter>(x)
    {}
    Iter begin() const {return this->first;}
    Iter end()   const {return this->second;}
  };
  template<class Iter>
  inline iter_pair_range<Iter> as_range(std::pair<Iter,Iter> const& x)
  { return iter_pair_range<Iter>(x); }
}

namespace tree_decomposition {
  typedef unsigned int uint;
  typedef smallset<uint> vertex_list;
  typedef std::vector<std::pair<uint, uint> > edge_list;

  struct bag;
  typedef std::shared_ptr<bag> bag_ptr;
  struct bag {
    vertex_list vertices;
    edge_list edges;
    std::vector<bag_ptr> children;
  };

  using tree_decomposition = bag_ptr;

  template<class Graph, class Range>
  bag_ptr build_tree_decomposition(Range const& order, Graph g)
  // please note that g is passed by copy
  {
    using namespace boost;
    using vertex_descriptor = typename graph_traits<Graph>::vertex_descriptor;

    std::vector<vertex_descriptor> vertices(num_vertices(g));
    auto free_edge_index = num_edges(g);

    // list (stable) vertex descriptors in the user-supplied order
    uint i = 0;
    for (auto vi : order)
      vertices[i++] = vertex(vi, g);

    // bags will be indexed by vertex indices
    std::vector<bag_ptr> bags(num_vertices(g));

    // each bag's parent index need also to be stored
    std::map<uint, uint> parent;

    // we need this map to differenciate between edges added during
    // vertex eliminations from the ones originally present in the
    // graph
    auto filling_edge = make_vector_property_map<bool>(get(edge_index, g));

    // iterate over these vertices in order
    for (auto v : vertices) {
      // this is the index
      uint vi = get(vertex_index, g, v);

      bags[vi] = std::make_shared<bag>();

      // add to each vertex bag the vertex (index) vi
      bags[vi]->vertices.insert(vi);
      for (auto e : as_range(out_edges(v, g))) {
        // and add its neighbours
        uint ui = get(vertex_index, g, target(e, g));
        bags[vi]->vertices.insert(ui);
        // add the edge to the bag if the edge is 'genuine'
        if (not filling_edge[e])
          bags[vi]->edges.push_back(std::make_pair(vi, ui));
      }

      // if we find the parent vertex index
      if (out_degree(v, g) > 0) {
        auto u = *find_first_of(vertices, adjacent_vertices(v, g));
        parent[vi] = get(vertex_index, g, u);
      }

      // eliminate vertex, marking filling edges as we add them
      for (auto a : as_range(adjacent_vertices(v, g))) {
        for (auto b : as_range(adjacent_vertices(v, g))) {
          if (a != b and not edge(a, b, g).second) {
            auto e = add_edge(a, b, free_edge_index++, g).first;
            filling_edge[e] = true;
          }
        }
      }
      clear_vertex(v, g);
      remove_vertex(v, g);
    }

    // ok, now we have a list of bags each with a parent index
    // time to unroll the tree

    for (auto i : order | adaptors::reversed) {
      if (parent.find(i) != parent.end())
        bags[parent[i]]->children.push_back(bags[i]);
    }

    return bags[order.back()];
  }

  unsigned int max_bag_size(tree_decomposition t)
  {
    auto max = t->vertices.size();
    for (auto b : t->children) {
      auto m = max_bag_size(b);
      if (m > max)
        max = m;
    }
    return max;
  }

  std::ostream& operator<<(std::ostream& o, tree_decomposition t)
  {
    o << "( ";
    for (auto v : t->vertices) {
      o << v << " ";
    }
    if (not t->edges.empty()) {
      o << "| ";
      for (auto p : t->edges) {
        o << p.first << "-" << p.second << " ";
      }
    }
    o << ") ";
    if (t->children.size()) {
      o << "{ ";
      for (auto b : t->children)
        o << b;
      o << "} ";
    }
    return o;
  }
}

#endif
