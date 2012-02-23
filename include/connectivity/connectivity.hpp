/*
 *  connectivity.hpp
 *
 *
 *  Created by Andrea Bedini on 10/Jul/2009.
 *  Copyright 2009, 2010, 2011, 2012 Andrea Bedini.
 *
 *  Distributed under the terms of the GNU General Public License.
 *  The full license is in the file COPYING, distributed as part of
 *  this software.
 *
 */

#ifndef CONNECTIVITY_HPP
#define CONNECTIVITY_HPP

#include "connectivity_functions.hpp"

#include <boost/cstdint.hpp>
#include <boost/functional/hash.hpp>

#include <algorithm>
#include <iosfwd>

class connectivity
{
  typedef boost::uint8_t uint8_t;

  unsigned int size_;
  uint8_t * impl_;

public:
  connectivity(unsigned int n)
    : size_(n)
    , impl_(new uint8_t[size_])
  {
    for (unsigned int i = 0; i < size_; ++i)
      impl_[i] = i + 1;
  }

  connectivity(const connectivity& rhs)
    : size_(rhs.size_)
    , impl_(new uint8_t[size_])
  {
    std::copy(rhs.impl_, rhs.impl_ + rhs.size_, impl_);
  }

  ~connectivity()
  {
    if (impl_)
      delete [] impl_;
  }

  connectivity& operator=(const connectivity& rhs)
  {
    if (this != &rhs) {
      uint8_t* new_impl_ = new uint8_t[rhs.size_];
      std::copy(rhs.impl_, rhs.impl_ + rhs.size_, new_impl_);
      delete [] impl_;
      impl_ = new_impl_;
      size_ = rhs.size_;
    }
    return *this;
  }

  unsigned int size() const
  {
    return size_;
  }

  void swap(connectivity& rhs) throw ()
  {
    using std::swap;
    swap(size_, rhs.size_);
    swap(impl_, rhs.impl_);
  }

  bool operator==(const connectivity& rhs) const
  {
    return (size_ == rhs.size_) and std::equal(impl_, impl_ + size_, rhs.impl_);
  }

  connectivity& canonicalize()
  {
    connectivity_functions::canonicalize(size_, impl_);
    return *this;
  }

  //////////////////////////////////////////////////////////////////////
  // const
  //////////////////////////////////////////////////////////////////////

  bool connected(unsigned int i, unsigned int j) const
  {
    return connectivity_functions::connected(impl_, i, j);
  }

  bool singleton(unsigned int i) const
  {
    return connectivity_functions::is_singleton(size_, impl_, i);
  }

  connectivity& delete_node(unsigned int i)
  {
    connectivity new_(size_ - 1);
    std::copy(impl_, impl_ + i, new_.impl_);
    std::copy(impl_ + i + 1, impl_ + size_, new_.impl_ + i);
    swap(new_);
    return *this;
  }

  connectivity& connect(unsigned int i, unsigned int j)
  {
    connectivity_functions::connect(size_, impl_, i, j);
    return *this;
  }

  connectivity& detach(unsigned int i)
  {
    connectivity_functions::detach(size_, impl_, i);
    return *this;
  }

  template<typename F>
  void decompose(F f) const
  {
    connectivity_functions::decompose(size_, impl_, f);
  }

  friend std::size_t hash_value(const connectivity&);
  friend std::ostream& operator<<(std::ostream&, const connectivity&);
};

std::size_t hash_value(const connectivity& a)
{
  return boost::hash_range(a.impl_, a.impl_ + a.size_);
}

std::ostream& operator<<(std::ostream& o, const connectivity& c)
{
  o << "{ ";
  for (unsigned int i = 0; i < c.size_; ++i) {
    o << (int) c.impl_[i] << " ";
  }
  return o << "}";
}

#endif
