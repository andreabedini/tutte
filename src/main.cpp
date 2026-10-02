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
#include <boost/iterator/counting_iterator.hpp>
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
  return range::equal(sort(range), irange((size_t) 0, num_vertices(g)));
}

/*
 *  The algorithm to run, dependent on the weight type
 */

template<typename T>
using algo = tutte<polynomial_two<T>>;

struct graph_component {
  graph_type graph;
  std::vector<unsigned int> local_to_global;
};

template<class ComponentMap>
std::vector<graph_component> extract_components(graph_type const& g,
  ComponentMap component_map, int num_components,
  std::vector<int>& component_by_vertex,
  std::vector<unsigned int>& global_to_local)
{
  std::vector<graph_component> result(num_components);
  auto global_vertex_index = get(boost::vertex_index, g);
  component_by_vertex.resize(num_vertices(g));
  global_to_local.resize(num_vertices(g));

  graph_type::vertex_iterator vi, vi_end;
  for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    auto const global = get(global_vertex_index, *vi);
    auto const current_component = get(component_map, *vi);
    component_by_vertex[global] = current_component;
    global_to_local[global] = result[current_component].local_to_global.size();
    result[current_component].local_to_global.push_back(global);
  }

  std::vector<std::vector<std::pair<unsigned int, unsigned int> > > edge_lists(
    num_components);
  graph_type::edge_iterator ei, ei_end;
  for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    auto source_v = source(*ei, g);
    auto target_v = target(*ei, g);
    auto const current_component = get(component_map, source_v);
    edge_lists[current_component].push_back(std::make_pair(
      global_to_local[get(global_vertex_index, source_v)],
      global_to_local[get(global_vertex_index, target_v)]));
  }

  for (int i = 0; i < num_components; ++i) {
    boost::counting_iterator<int> edge_index(0);
    result[i].graph = graph_type(edge_lists[i].begin(), edge_lists[i].end(),
      edge_index, result[i].local_to_global.size());

    unsigned int j = 0;
    for (tie(vi, vi_end) = vertices(result[i].graph); vi != vi_end; ++vi)
      put(boost::vertex_index, result[i].graph, *vi, j++);
  }

  return result;
}

template<class OutputIterator>
void compute_order(graph_type const& g, boost::program_options::variables_map const& vm,
  OutputIterator out)
{
  if (vm.count("fill-in")) {
    heuristics::greedy_fillin_order(g, out);
  } else if (vm.count("local-degree")) {
    heuristics::greedy_local_degree_order(g, out);
  } else if (vm.count("local-fill-in")) {
    heuristics::greedy_local_fillin_order(g, out);
  } else {
    heuristics::greedy_degree_order(g, out);
  }
}

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
    ("Q,Q", po::value<int32_t>(), "Fix Q value, to be used with v")
    ("v,v", po::value<int32_t>(), "Fix v value, to be used with Q")
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

  auto component = boost::make_vector_property_map<int>(
    get(boost::vertex_index, g));
  auto num_components = connected_components(g, component);

  std::vector<int> component_by_vertex(num_vertices(g));
  std::vector<unsigned int> global_to_local(num_vertices(g));
  auto components = extract_components(g, component, num_components,
    component_by_vertex, global_to_local);

  std::vector<unsigned int> user_order;
  std::vector<std::vector<unsigned int> > orders(num_components);
  if (vm.count("elimination-order")) {
    user_order.resize(num_vertices(g));
    std::string s = vm["elimination-order"].as<std::string>();
    parse_elimination_order(s, user_order.begin());
    bool valid = validate_elimination_order(user_order, g);
    if (not valid) {
      std::cerr << "error: elimination order not valid\n";
      return 1;
    }
    for (int i = 0; i < num_components; ++i)
      orders[i].reserve(components[i].local_to_global.size());

    std::vector<bool> seen_component(num_components, false);
    int current_component = -1;
    for (auto v : user_order) {
      auto const next_component = component_by_vertex[v];
      if (next_component != current_component) {
        if (seen_component[next_component]) {
          std::cerr << "error: elimination order must keep connected components contiguous\n";
          return 1;
        }
        seen_component[next_component] = true;
        current_component = next_component;
      }
      orders[next_component].push_back(global_to_local[v]);
    }

    for (int i = 0; i < num_components; ++i) {
      if (not validate_elimination_order(orders[i], components[i].graph)) {
        std::cerr << "error: elimination order not valid\n";
        return 1;
      }
    }
    std::cerr << "Vertex ordering: " << s << "\n";
  }

  std::vector<tree_decomposition::bag_ptr> decompositions;
  decompositions.reserve(num_components);
  for (int i = 0; i < num_components; ++i) {
    if (not vm.count("elimination-order")) {
      orders[i].resize(components[i].local_to_global.size());
      compute_order(components[i].graph, vm, orders[i].begin());
    }

    decompositions.push_back(tree_decomposition::build_tree_decomposition(
      orders[i], components[i].graph));
  }

  if (vm.count("print-tree") or vm.count("tree-only")) {
    if (num_components == 1) {
      std::cerr << "Elimination order: ";
      for (auto x : orders.front())
        std::cerr << components.front().local_to_global[x] << " ";
      std::cerr << "\n";

      std::cerr << "Tree decomposition: ";
      tree_decomposition::print(std::cerr, decompositions.front(),
        components.front().local_to_global);
      std::cerr << "\n"
                << "Tree decomposition width: "
                << max_bag_size(decompositions.front()) - 1 << "\n";
    } else {
      for (std::size_t i = 0; i < components.size(); ++i) {
        std::cerr << "Component " << i + 1 << " elimination order: ";
        for (auto x : orders[i])
          std::cerr << components[i].local_to_global[x] << " ";
        std::cerr << "\n";

        std::cerr << "Component " << i + 1 << " tree decomposition: ";
        tree_decomposition::print(std::cerr, decompositions[i],
          components[i].local_to_global);
        std::cerr << "\n"
                  << "Component " << i + 1 << " tree decomposition width: "
                  << max_bag_size(decompositions[i]) - 1 << "\n";
      }
    }
  }

  if (vm.count("tree-only"))
    return 0;

  if (vm.count("Q") && vm.count("v")) {
    std::cerr << "Running with fixed values of Q and v\n";
    auto Q = vm["Q"].as<int32_t>();
    auto v = vm["v"].as<int32_t>();
    auto result = chinese_remainder::chinese_remainder_result<tutte>(
      decompositions.front(), Q, v);
    for (std::size_t i = 1; i < decompositions.size(); ++i)
      result *= chinese_remainder::chinese_remainder_result<tutte>(
        decompositions[i], Q, v);
    std::cout << result << "\n";
  } else {
    auto Q = polynomial_two<int>::Q();
    auto v = polynomial_two<int>::v();

    if (vm.count("flow")) {
      v = -Q;
    } else if (vm.count("chromatic")) {
      v = -1;
    }

    if (vm.count("chinese-remainder")) {
      auto result = chinese_remainder::chinese_remainder_result<algo>(
        decompositions.front(), Q, v);
      for (std::size_t i = 1; i < decompositions.size(); ++i)
        result *= chinese_remainder::chinese_remainder_result<algo>(
          decompositions[i], Q, v);
      std::cout << result << "\n";
    } else {
      using gmp::mpz_int;
      auto result = polynomial_two<mpz_int>(1);
      for (auto const& decomposition : decompositions)
        result *= transfer::transfer(algo<mpz_int>(Q, v), decomposition);
      std::cout << result << "\n";
    }
  }
}
