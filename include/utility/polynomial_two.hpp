/*
 *  polynomial_two_two.hpp
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

#ifndef POLYNOMIAL_TWO_HPP
#define POLYNOMIAL_TWO_HPP

#include <boost/operators.hpp>
#include <boost/format.hpp>
#include <iosfwd>
#include <map>

template<class T>
class polynomial_two
  : boost::ring_operators1< polynomial_two<T>
  , boost::ring_operators2< polynomial_two<T>, T
  , boost::equality_comparable< polynomial_two<T>
  > > >
{
public:
  typedef unsigned short int index;

  struct element {
    index i, j;
    T c;
    element(index i, index j, T c)
      : i(i), j(j), c(c) {}
    bool operator==(element const& rhs) const
    { return i == rhs.i and j == rhs.j and c == rhs.c; }
  };

private:
  typedef std::vector<element> elements_type;
  elements_type elements_;

public:
  typedef typename elements_type::iterator iterator;
  typedef typename elements_type::const_iterator const_iterator;

private:
  void cleanup() {
    iterator i = begin();
    while (i != end()) {
      if (i->c == T(0))
	i = elements_.erase(i);
      else
	++i;
    }
  }

  struct indices_less {
    bool operator()(element const& a, element const& b) const
    {
      return a.i < b.i or (a.i == b.i and a.j < b.j);
    }
  };

  struct indices_equal {
    bool operator()(element const& a, element const& b) const
    {
      return a.i == b.i and a.j == b.j;
    }
  };

  T& coeff(index const& i, index const& j)
  {
    const element e(i, j, T(0));
    iterator it = std::lower_bound(begin(), end(), e, indices_less());
    if (it == end() or not indices_equal()(*it, e))
      it = elements_.insert(it, e);
    return it->c;
  }

  polynomial_two(elements_type const& e) : elements_(e)
  { }

public:
  // default constructor
  explicit polynomial_two(T const& a = T(0))
    : elements_(1, element(0, 0, a))
  {
  }
  
  // conversion
  template<class T2>
  explicit polynomial_two(T2 const& a)
    : elements_(1, element(0, 0, T(a)))
  {
  }

  template<class T2>
  explicit polynomial_two(polynomial_two<T2> const& rhs)
  {
    typename polynomial_two<T2>::const_iterator it;
    for (it = rhs.begin(); it != rhs.end(); ++it)
      coeff(it->i, it->j) = T(it->c);
  }

  // assignemnt
  template<class T2>
  polynomial_two<T>& operator=(T2 const& rhs)
  {
    elements_.clear();
    elements_.push_back(element(0, 0, T(rhs)));
    return *this;
  }

  template<class T2>
  polynomial_two<T>& operator=(polynomial_two<T2> const& rhs)
  {
    elements_.clear();
    typename polynomial_two<T2>::const_iterator it;
    for (it = rhs.begin(); it != rhs.end(); ++it)
      coeff(it->i, it->j) = T(it->c);
    return *this;
  }

  // static constructors
  
  static polynomial_two<T> Q()
  {
    return polynomial_two(elements_type(1, element(1, 0, T(1))));
  }

  static polynomial_two<T> v()
  {
    return polynomial_two(elements_type(1, element(0, 1, T(1))));
  }

  // swap
  void swap(polynomial_two<T>& other) throw ()
  {
    elements_.swap(other.elements_);
  }
  
  // iterators
  iterator begin() { return elements_.begin(); }
  iterator end()   { return elements_.end(); }

  const_iterator begin() const { return elements_.begin(); }
  const_iterator end()   const { return elements_.end(); }

   // comparison
  bool operator==(polynomial_two<T> const& rhs) const
  {
    return std::equal(begin(), end(), rhs.begin());
  }
  
  // ring operators with T
  polynomial_two<T>& operator+=(T const& rhs)
  {
    coeff(0, 0) += rhs;
    return *this;
  }
  
  polynomial_two<T>& operator-=(T const& rhs)
  {
    coeff(0, 0) -= rhs;
    return *this;
  }
  
  polynomial_two<T>& operator*=(T const& rhs)
  {
    iterator it;
    for (it = begin(); it != end(); ++it)
      it->c *= rhs;
    return *this;
  }

  polynomial_two<T>& operator%=(T const& rhs)
  {
    iterator it;
    for (it = begin(); it != end(); ++it)
      it->c %= rhs;
    return *this;
  }
  
  // ring operators with polynomial_two<T>
  polynomial_two<T>& operator+=(polynomial_two<T> const& rhs)
  {
    const_iterator it;
    for (it = rhs.elements_.begin(); it != rhs.elements_.end(); ++it)
      coeff(it->i, it->j) += it->c;
    cleanup();
    return *this;
  }
  
  polynomial_two<T>& operator-=(polynomial_two<T> const& rhs)
  {
    const_iterator it;
    for (it = rhs.elements_.begin(); it != rhs.elements_.end(); ++it)
      coeff(it->i, it->j) -= it->c;
    cleanup();
    return *this;
  }
  
  polynomial_two<T>& operator*=(polynomial_two<T> const& rhs)
  {
    polynomial_two<T> result;
    const_iterator it1, it2;
    for (it1 = rhs.elements_.begin(); it1 != rhs.elements_.end(); ++it1) {
      for (it2 = begin(); it2 != end(); ++it2) {
    	result.coeff(it1->i + it2->i, it1->j + it2->j) += it1->c * it2->c;
      }
    }
    swap(result);
    return *this;
  }

  // unary

  const polynomial_two<T> operator-() const
  {
    polynomial_two<T> y;
    const_iterator it;
    for (it = begin(); it != end(); ++it) {
      y.coeff(it->i, it->j) = -it->c;
    }
    return y;
  }

  // member functions
  
  const polynomial_two<T> times_Q() const
  {
    polynomial_two<T> result;
    const_iterator it;
    for (it = begin(); it != end(); ++it)
      result.coeff(it->i + 1, it->j) = it->c;
    return result;
  }

  const polynomial_two<T> times_v() const
  {
    polynomial_two<T> result;
    const_iterator it;
    for (it = begin(); it != end(); ++it)
      result.coeff(it->i, it->j + 1) = it->c;
    return result;
  }
  
  friend
  std::ostream& operator<<(std::ostream& o, polynomial_two<T> const& p)
  {
    const_iterator it = p.elements_.begin();
    while (it != p.elements_.end()) {
      T c = it->c;
      if (c < 0) {
	o << "- ";
	c = -c;
      } else {
	o << "+ ";
      }
      if (c != 1 or (it->i == 0 and it->j == 0))
	o << c << " ";
      if (it->i == 1)
	o << "Q ";
      if (it->i > 1)
	o << "Q^" << it->i << " ";
      if (it->j == 1)
	o << "v ";
      if (it->j > 1)
	o << "v^" << it->j << " ";
      ++ it;
    }
    return o;
  }		
};

template<class T>
void swap(polynomial_two<T>& p1, polynomial_two<T>& p2) throw ()
{
  p1.swap(p2);
}

#endif // POLYNOMIAL_TWO_HPP
