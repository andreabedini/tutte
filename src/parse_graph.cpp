/*
 *  parse_graph.cpp
 *
 *
 *  Created by Andrea Bedini on 24/Nov/2011.
 *  Copyright 2011, 2012 Andrea Bedini.
 *
 *  Distributed under the terms of the GNU General Public License.
 *  The full license is in the file COPYING, distributed as part of
 *  this software.
 *
 */

#include "graph_type.hpp"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/counting_iterator.hpp>

#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

graph_type parse_graph(std::string const& s)
{
  using namespace std;
  using namespace boost;

  vector<pair<int, int> > edge_list;

  string parse_err_msg = "parse error";
  istringstream iss(s);

  if (!iss.good())
    throw runtime_error(parse_err_msg + " 1");

  unsigned int n = 0;
  while (true) {
    unsigned int a = 0, b = 0;

    iss >> a;
    n = max(n, a);

    if (iss.get() !=  '-' or iss.bad())
      throw runtime_error("expecting a dash!");
    if (iss.get() !=  '-' or iss.bad())
      throw runtime_error("expecting a dash!");

    iss >> b;
    n = max(n, b);

    edge_list.push_back(make_pair(a, b));

    if (iss.eof())
      break;

    if (iss.get() !=  ',' or iss.bad())
      throw runtime_error("expecting a comma!");
  }

  boost::counting_iterator<int> ep_iter(0);
  graph_type g(edge_list.begin(), edge_list.end(), ep_iter, n + 1);

  unsigned int i = 0;
  graph_type::vertex_iterator vi, vi_end;
  for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
    put(vertex_index, g, *vi, i++);

  return g;
}
