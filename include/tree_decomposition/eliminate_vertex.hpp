/*
 *  eliminate_vertex.hpp
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

#ifndef ELIMINATE_VERTEX_HPP
#define ELIMINATE_VERTEX_HPP

#include <boost/foreach.hpp>
#include <boost/graph/graph_traits.hpp>

template<class Vertex, class Graph>
void eliminate_vertex(Vertex v, Graph& g) {
  BOOST_FOREACH(Vertex a, adjacent_vertices(v, g)) {
    BOOST_FOREACH(Vertex b, adjacent_vertices(v, g)) {
      if (a != b and not edge(a, b, g).second) {
	add_edge(a, b, g);
      }
    }
  }
  clear_vertex(v, g);
  remove_vertex(v, g);
}

#endif
