/*
 *  Zp.hpp
 *
 *  Created by Andrea Bedini on 19/09/08.
 *  Copyright (c) 2008-2013, Andrea Bedini <andrea.bedini@gmail.com>.
 *
 *  Distributed under the terms of the Modified BSD License.
 *  The full license is in the file COPYING, distributed as part of
 *  this software.
 *
 */

#ifndef ZP_HPP
#define ZP_HPP

#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_signed.hpp>
#include <boost/utility/enable_if.hpp>

#include <iosfwd>

namespace modular {
  class Zp;
}

namespace boost {
  template<> struct is_integral <modular::Zp> : public true_type {};
  template<> struct is_signed   <modular::Zp> : public false_type {};
  template<> struct is_unsigned   <modular::Zp> : public true_type {};
}

namespace modular {
  using boost::enable_if;
  using boost::disable_if;

  using boost::uint64_t;

  typedef long double ldouble;

  inline uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t m)
  {
    uint64_t x = a * b;
    uint64_t y = m * (uint64_t)( (ldouble)a * (ldouble)b/m + (ldouble)1/2 );
    uint64_t r = x - y;
    if ( (int64_t)r < 0 )  r += m;
    return  r;
  }

  class Zp : boost::ring_operators< Zp
	   , boost::equality_comparable< Zp
	   > >
  {
    static uint64_t M;
    uint64_t rep_;

  public:
    Zp() : rep_(0) { }
    Zp(Zp const& x) : rep_(x.rep_) { }

    template<class T>
    explicit Zp(T n, typename boost::enable_if<boost::is_signed<T> >::type* = 0)
      : rep_( n < 0 ?  M - ((- n) % M) : n % M)
    { }

    template<class T>
    explicit Zp(T n, typename boost::disable_if<boost::is_signed<T> >::type* = 0)
      : rep_(n % M)
    { }

    operator unsigned long() const { return rep_; }

    static uint64_t get_modulus() {
      return M;
    }

    static void set_modulus(uint64_t p) {
      M = p;
    }

    // addable
    Zp& operator+=(Zp const& x)
    {
      if (rep_ >= M - x.rep_)
	rep_ -= M - x.rep_;
      else
	rep_ += x.rep_;
      return *this;
    }

    // subtractable
    Zp& operator-=(Zp const& x)
    {
      if (rep_ >= x.rep_)
	rep_ -= x.rep_;
      else
	rep_ = M - x.rep_ + rep_;
      return *this;
    }

    // multipliable
    Zp& operator*=(Zp const& x)
    {
      rep_ = mul_mod(rep_, x.rep_, M);
      // rep_ *= x.rep_;
      // rep_ %= M;
      return *this;
    }

    Zp operator-() const
    {
      return Zp(M - rep_);
    }

    // equality_comparable
    bool operator==(Zp const& rhs) const
    {
      return rep_ == rhs.rep_;
    }

    friend std::ostream& operator<<(std::ostream& o, Zp const& x)
    {
      return o << x.rep_ << " (" << M << ")";
    }
  };

  uint64_t Zp::M;
}

#endif
