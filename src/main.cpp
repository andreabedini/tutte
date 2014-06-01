/*
 *  tutte.cpp
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

#include "chinese_remainder.hpp"
#include "graph_type.hpp"
#include "parse_graph.hpp"
#include "transfer.hpp"
#include "tree_decomposition/heuristics.hpp"
#include "tree_decomposition/tree_decomposition.hpp"
#include "tutte.hpp"
#include "utility/gmp.hpp"
#include "utility/polynomial_two.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/property_map/vector_property_map.hpp>
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
  "tutte - computes the Tutte Polynomial\n"
  "Copyright (c) 2011-2014, Andrea Bedini <andrea.bedini@gmail.com>.\n\n"
  "Distributed under the terms of the Modified BSD License.\n"
  "The full license is in the file COPYING, distributed as part of\n"
  "this software.\n";

/*
 * code to parse user-supplied elimination order
 */

template<class OutputIterator>
void parse_elimination_order(std::string const& s, OutputIterator out)
{
  boost::tokenizer<> tok(s);
  for (auto& c : tok)
    *out++ = boost::lexical_cast<unsigned>(c);
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

/*
 *  The algorithm to run, dependent on the weight type 
 */

template<typename T>
using algo = tutte<polynomial_two<T>>;

int main (int argc, char *argv[])
{
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "Produce help message")
    ("input-file", po::value<std::string>(), "Read the graph from a file.")
    // tree decomposition options
    ("degree", "Use greedy degree algorithm [default].")
    ("fill-in", "Use greedy fill-in algorithm.")
    ("local-degree", "Use 'local' greedy degree algorithm.")
    ("local-fill-in", "Use 'local' greedy fill-in algorithm.")
    ("elimination-order", po::value<std::string>(), "Specify a vertex elimination order.")
    ("print-tree", "Print tree decomposition.")
    ("tree-only", "Print tree decomposition and exit.")
    // tutte options
    ("flow,f", "Compute the flow polynomial")
    ("chromatic,c", "Compute the chromatic polynomial")
    ("chinese-remainder", "Use the chinese remainder trick.")
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
    std::cerr << manifesto << "\n" << desc << "\n";
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

  // check for connectedness
  auto component = boost::make_vector_property_map<int>(
    get(boost::vertex_index, g));
  auto num = connected_components(g, component);
  if (num > 1) {
    std::cerr << "The input graph is not connected. A connected input is required\n";
    return 1;
  }

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

  auto td = tree_decomposition::build_tree_decomposition(order, g);

  if (vm.count("print-tree") or vm.count("tree-only")) {
    std::cerr << "Elimination order: ";
    for (auto x : order)
      std::cerr << x << " ";
    std::cerr << "\n";

    std::cerr << "Tree decomposition: " << td << "\n"
              << "Tree decomposition width: "
              << max_bag_size(td) - 1 << "\n";
  }

  if (vm.count("tree-only"))
    return 0;

  auto Q = polynomial_two<int>::Q();
  auto v = polynomial_two<int>::v();

  if (vm.count("flow")) {
    v = -Q;
  } else if (vm.count("chromatic")) {
    v = -1;
  }

  if (vm.count("chinese-remainder")) {
    chinese_remainder::chinese_remainder<algo>(td, Q, v);
  } else {
    using gmp::mpz_int;
    auto result = transfer::transfer(algo<mpz_int>(Q, v), td);
    std::cout << result << "\n";
  }
}
