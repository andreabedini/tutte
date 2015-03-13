[![DOI](https://zenodo.org/badge/7975/andreabedini/tutte.svg)](http://dx.doi.org/10.5281/zenodo.15941)
[![Build Status](https://travis-ci.org/andreabedini/tutte.svg?branch=travis_tests)](https://travis-ci.org/andreabedini/tutte)

# Tutte

This program computes the Tutte polynomial of a user-supplied graph
using the algorithm described in:

> Bedini, A. & Jacobsen, J.L. A tree-decomposed transfer matrix for
> computing exact Potts model partition functions for arbitrary graphs,
> with applications to planar graph colourings. J. Phys. A: Math. Theor.
> 43, 385001 (2010). [dx.doi.org/10.1088/1751-8113/43/38/385001](http://dx.doi.org/10.1088/1751-8113/43/38/385001).

## Requirements

  - A modern C++ compiler which supports C++11.

  - gmplib, http://gmplib.org

  - Boost libraries, http://www.boost.org

## Compilation and installation

    $ git clone git://github.com/andreabedini/tutte.git
    $ cd tutte
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

  You can also run some tests to make sure everything is working properly.

    $ make test

  Optionally, you can install the executable program to your path.

    $ make install

## Input specification

The input is expected to be passed to the program through the standard input. The result will be printed to the standard output while any additional message will be printed on the standard error stream.

E.g. you can do

    $ cat my_graph | bin/tutte

  or

    $ bin/tutte < my_graph

The input format looks like this:

0--1,1--2,2--3,3--0,4--5,5--6,6--7,7--4

This describe a graph with 8 vertices and 8 edges. Each edge is represented as a pair of numbers each representing a vertex.

It is _important_ that the vertex numbering starts from zero, so that vertex indices are in [0..V) where V is the number of vertices.

Alternatively an input file can be specified with the option --input-file

    $ bin/tutte --input-file my_input

NOTE: The input graph has to be connected, giving as input a graph with multiple connected components will result in a run-time error. This can be easily solved but I haven't had the time to work on it yet. Drop a line on the [bug report](https://github.com/andreabedini/tutte/issues/2) if you feel like helping out.

## Options

A list of options is available with the `--help` command

    Allowed options:
      -h [ --help ]           Produce help message
      --input-file arg        Read the graph from a file.
      --degree                Use greedy degree algorithm [default].
      --fill-in               Use greedy fill-in algorithm.
      --local-degree          Use 'local' greedy degree algorithm.
      --local-fill-in         Use 'local' greedy fill-in algorithm.
      --elimination-order arg Specify a vertex elimination order.
      --print-tree            Print tree decomposition.
      --tree-only             Print tree decomposition and exit.
      -f [ --flow ]           Compute the flow polynomial
      -c [ --chromatic ]      Compute the chromatic polynomial
      --chinese-remainder     Use the chinese remainder trick.

The options `--flow` and `--chromatic` tell the program to compute the
relevant specialization of the Tutte polynomial. In the variables (Q, v), passing `--flow` sets v = -Q, while passing `--chromatic` sets v = -1.

The options `--degree` and `--fillin` choose which algorithm has to be used to compute the tree decomposition. The algorithms Greedy Degree and Greedy Fill-In  are described in

> Bodlaender, H.L. & Koster, A.M.C.A. Treewidth computations I. Upper
> bounds. Information and Computation 208, 259–275 (2010). [10.1016/j.ic.2009.03.008](http://dx.doi.org/10.1016/j.ic.2009.03.008).

The algorithms _local_ greedy degree and _local_ greedy fill-in algorithm are home-crafted modifications to the above to make them always output a path-decomposition.

A vertex elimination order (see Bodlaender & Koster (2010) for the terminology) can be specified on directly on the command line as a comma separated list of vertices.

## Remarks

Edges are assigned to bags as they appear in the elimination ordering. For the sake of generality and maintenance, problem specific optimizations, such as the pruning procedure described in Bedini & Jacobsen (2010), are not implemented.

