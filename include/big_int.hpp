/*
 *  big_int.hpp
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

#ifndef BIG_INT_HPP
#define BIG_INT_HPP

#include <boost/type_traits/is_signed.hpp>
#include <gmpxx.h>

typedef mpz_class big_int;

mpz_class modinv(const mpz_class& a, const mpz_class& b)
{
  mpz_class r;
  mpz_invert(r.get_mpz_t(), a.get_mpz_t(), b.get_mpz_t());
  return r;
}

namespace boost {
  template <>
  struct is_signed<big_int> : public true_type { };
}

#endif
