/*
 *  tutte.cpp
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

#include "tutteconfig.h"

#include "big_int.hpp"
#include "chinese_compute.hpp"
#include "graph_type.hpp"
#include "parse_graph.hpp"
#include "transfer.hpp"
#include "tree_decomposition/heuristics.hpp"
#include "tree_decomposition/tree_decomposition.hpp"
#include "tutte.hpp"
#include "utility/polynomial_two.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const char manifesto[] =
  "tutte - computes the Tutte Polynomial - "
  "version " VERSION "\n\n"
  "Copyright 2011 Andrea Bedini.\n\n"
  "Distributed under the terms of the GNU General Public License.\n"
  "The full license is in the file COPYING, distributed as part of\n"
  "this software.\n";

/*
 * code to parse user supplied elimination order
 */

template<class OutputIterator>
void parse_elimination_order(std::string const& s, OutputIterator out)
{
  boost::tokenizer<> tok(s);
  boost::tokenizer<>::iterator i;
  for(i = tok.begin(); i != tok.end(); ++i)
    *out++ = boost::lexical_cast<unsigned>(*i);
}

/*
 *  validate the ordering by checking if it's a permutation of [0..num_vertices)
 */
template<class Range, class Graph>
bool validate_elimination_order(Range range, Graph const& g)
{
  using namespace boost;
  return equal(sort(range), irange((size_t) 0, num_vertices(g)));
}

int main (int argc, char *argv[])
{
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "Produce help message")
    ("flow,f", "Compute the flow polynomial")
    ("chromatic,c", "Compute the chromatic polynomial")
    ("degree", "Use greedy degree algorithm [default].")
    ("fill-in", "Use greedy fill-in algorithm.")
    ("local-degree", "Use 'local' greedy degree algorithm.")
    ("local-fill-in", "Use 'local' greedy fill-in algorithm.")
    ("elimination-order", po::value<std::string>(), "Specify a vertex elimination order.")
    ("print-tree", "Print tree decomposition.")
    ("tree-only", "Print tree decomposition and exit.")
    ("chinese-remainder", "Use the chinese remainder trick.")
    ("input-file", po::value<std::string>(), "Read the graph from a file.")
    ;

  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch (po::error& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
  
  if (vm.count("help")) {
    std::cerr << manifesto << "\n";
    std::cerr << desc << "\n";
    return 0;
  }
  
  int check = 0;
  check += vm.count("degree");
  check += vm.count("fill-in");
  check += vm.count("local-degree");
  check += vm.count("local-fill-in");
  check += vm.count("elimination-order");

  if (check > 1) {
    std::cerr <<
      "error: please specify at most one between degree, fill-in,"
      "local-degree, local-fill-in and elimination-order\n";
    return 1;
  }
  
  graph_type g;
  try {
    std::string s;
    if (vm.count("input-file")) {
      std::string filename = vm["input-file"].as<std::string>();
      std::ifstream input(filename.c_str(), std::ios_base::in);
      if (not input.is_open()) {
	std::cerr << "error: file " << filename << " not found\n";
	return 1;
      }
      input >> s;
    } else {
      std::cin >> s;
    }
    g = parse_graph(s);
  } catch (std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
  
  std::cerr << "Graph with " << num_vertices(g) << " vertices and "
       << num_edges(g) << " edges.\n";
  
  std::vector<unsigned int> order(num_vertices(g));
  
  if (vm.count("fill-in")) {
    heuristics::greedy_fillin_order(g, order.begin());
  } else if (vm.count("local-degree")) {
    heuristics::greedy_local_degree_order(g, order.begin());
  } else if (vm.count("local-fill-in")) {
    heuristics::greedy_local_fillin_order(g, order.begin());
  } else if (vm.count("elimination-order")) {
    // parse the std::string
    std::string s = vm["elimination-order"].as<std::string>();
    parse_elimination_order(s, order.begin());
    bool valid = validate_elimination_order(order, g);
    if (not valid) {
      std::cerr << "error: elimination order not valid\n";
      return 1;
    }
    std::cerr << "Vertex ordering: " << s << "\n";
  } else {
    heuristics::greedy_degree_order(g, order.begin());
  }
  
  typedef tree_decomposition::tree_decomposition tree_type;
  tree_type td = tree_decomposition::build_tree_decomposition(order, g);
  
  if (vm.count("print-tree") or vm.count("tree-only")) {
    std::cerr << "Elimination order: ";
    for (unsigned int i = 0; i < num_vertices(g); ++i)
      std::cerr << order[i] << " ";
    std::cerr << "\n";
    
    std::cerr << "Tree decomposition: " << td << "\n"
	 << "Tree decomposition width: "
	 << tree_width(td) << "\n";
  }
  
  if (vm.count("tree-only"))
    return 0;
  
  polynomial_two<int> Q = polynomial_two<int>::Q();
  polynomial_two<int> v = polynomial_two<int>::v();
  
  if (vm.count("flow")) {
    v = -Q;
  } else if (vm.count("chromatic")) {
    v = -1;
  }
  
  if (vm.count("chinese-remainder")) {
    chinese_compute(td, Q, v);
  } else {
    typedef polynomial_two<big_int> big_poly;
    tutte<big_poly> tutte_algo(Q, v);
    big_poly result = transfer::transfer(tutte_algo, td);
    std::cout << result << "\n";
  }
}
