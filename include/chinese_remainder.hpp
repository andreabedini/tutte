/*
 *  chinese_remainder.hpp
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

#ifndef CHINESE_REMAINDER_HPP

#include "transfer.hpp"
#include "tree_decomposition/tree_decomposition.hpp"
#include "utility/gmp.hpp"
#include "utility/polynomial_two.hpp"
#include "utility/Zp.hpp"

#include <numeric>
#include <iostream>

namespace chinese_remainder {
  const uint32_t primes[] = {
    4294967291UL, 4294967279UL, 4294967231UL, 4294967197UL,
    4294967189UL, 4294967161UL, 4294967143UL, 4294967111UL,
    4294967087UL, 4294967029UL, 4294966997UL, 4294966981UL,
    4294966943UL, 4294966927UL, 4294966909UL, 4294966877UL,
    4294966829UL, 4294966813UL
  };

  const size_t num_primes = sizeof(primes)/sizeof(uint32_t);

  using gmp::mpz_int;

  using tree_decomposition::bag_ptr;

  template<template<class> class Algorithm, class... Args>
  void chinese_remainder(bag_ptr t, Args&&... args)
  {
    using modular::Zp;
    using big_t = typename Algorithm<mpz_int>::weight_type;

    unsigned int k = 0;
    big_t partial_results[num_primes];
    big_t result_last, result;
    mpz_int qs[num_primes];
    mpz_int pp = 1;

    do {
      Zp::set_modulus(primes[k]);
      Algorithm<Zp> algo(std::forward<Args>(args)...);

      pp *= primes[k];

      for (unsigned int i = 0; i < k + 1; ++ i)
        qs[i] = pp / primes[i] * modinv(pp / primes[i], primes[i]);

      result_last = result;

      partial_results[k] = big_t(transfer::transfer(algo, t));
      std::cerr << "result (mod " << Zp::get_modulus() << ")\t: " << partial_results[k] << "\n";

      result = std::inner_product(partial_results, partial_results + k + 1, qs, big_t(0));

      // this is specific to polynomial_two
      mpz_int limit = pp >> 1;
      for (auto& e : result) {
        e.c %= pp;
        if (e.c > limit)
          e.c -= pp;
      }

      ++ k;
    } while (result != result_last);
    std::cout << result << "\n";
  }
}
#endif
