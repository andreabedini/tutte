/*
 *  graph_type.hpp
 *
 *
 *  Created by Andrea Bedini on 24/Nov/2011.
 *  Copyright (c) 2011-2013, Andrea Bedini <andrea.bedini@gmail.com>.
 *
 *  Distributed under the terms of the Modified BSD License.
 *  The full license is in the file COPYING, distributed as part of
 *  this software.
 *
 */

#ifndef GRAPH_TYPE_HPP
#define GRAPH_TYPE_HPP

#include <boost/graph/adjacency_list.hpp>

typedef boost::adjacency_list
  < boost::listS, boost::listS, boost::undirectedS
  , boost::property<boost::vertex_index_t, unsigned int>
  , boost::property<boost::edge_index_t, unsigned int>
  > graph_type;

#endif
