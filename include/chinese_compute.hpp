/*
 *  chinese_compute.hpp
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

#ifndef CHINESE_COMPUTE_HPP

#include "big_int.hpp"
#include "transfer.hpp"
#include "tree_decomposition/tree_decomposition.hpp"
#include "tutte.hpp"
#include "utility/polynomial_two.hpp"
#include "utility/Zp.hpp"

#include <numeric>
#include <iostream>

const uint32_t primes[] = {
  4294967291UL, 4294967279UL, 4294967231UL, 4294967197UL,
  4294967189UL, 4294967161UL, 4294967143UL, 4294967111UL,
  4294967087UL, 4294967029UL, 4294966997UL, 4294966981UL,
  4294966943UL, 4294966927UL, 4294966909UL, 4294966877UL,
  4294966829UL, 4294966813UL
};

const size_t num_primes = sizeof(primes)/sizeof(uint32_t);

template<class T, class U>
void chinese_compute(const tree_decomposition::tree_decomposition& t,
		     const T& Q, const U& v)
{
  using namespace std;
  using namespace boost;

  using modular::Zp;
  
  typedef polynomial_two<big_int> big_poly;  
  typedef polynomial_two<Zp> small_poly;

  unsigned int k = 0;
  big_poly partial_results[num_primes];
  big_poly result_last, result;
  big_int qs[num_primes];
  big_int pp = 1;
  
  do {
    Zp::set_modulus(primes[k]);
    tutte<small_poly> tutte_algo(Q, v);

    pp *= primes[k];

    for (unsigned int i = 0; i < k + 1; ++ i)
      qs[i] = pp / primes[i] * modinv(pp / primes[i], primes[i]);
    
    result_last = result;
    
    partial_results[k] = big_poly(transfer::transfer(tutte_algo, t));
    cerr << "result (mod " << Zp::get_modulus() << ")\t: " << partial_results[k] << "\n";

    result = inner_product(partial_results, partial_results + k + 1, qs, big_poly(0));
    result %= pp;

    big_int limit = pp >> 1;
    for (big_poly::iterator i = result.begin(); i != result.end(); ++i)
      if (i->c > limit)
	i->c -= pp;

    ++ k;
  } while (result != result_last);
  cout << result << "\n";
}

#endif

