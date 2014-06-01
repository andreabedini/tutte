/*
 *  gmp.hpp
 *
 *
 *  Created by Andrea Bedini on 7/Jun/2013.
 *  Copyright (c) 2014, Andrea Bedini <andrea.bedini@gmail.com>.
 *
 *  Distributed under the terms of the Modified BSD License.
 *  The full license is in the file COPYING, distributed as part of
 *  this software.
 *
 */

#ifndef GMP_HPP
#define GMP_HPP

#include <iosfwd>
#include <type_traits>

namespace gmp {
	#include "gmp.h"

	class mpz_int {
		mpz_t m_data;
	public:
		// 5.1 Initialization Functions

		mpz_int()   { mpz_init(m_data); }
		~mpz_int() { mpz_clear(m_data); }

		mpz_int(mpz_int const& o) {
			mpz_init_set(m_data, o.m_data);
		}

		mpz_int& operator=(mpz_int const& o) {
			mpz_init_set(m_data, o.m_data);
			return *this;
		}

		// 5.3 Combined Initialization and Assignment Functions

		template<typename T>
		mpz_int(T op, typename std::enable_if<std::is_unsigned<T>::value >::type* = 0) {
			mpz_init_set_ui(m_data, op);
		}

		template<typename T>
		mpz_int(T op, typename std::enable_if<std::is_signed<T>::value >::type* = 0) {
			mpz_init_set_si(m_data, op);
		}

		mpz_int(double op) {
			mpz_init_set_d(m_data, op);
		}

		mpz_int(std::string const& op, int base = 10) {
			int ret = mpz_init_set_str(m_data, op.c_str(), base);
			if (ret == -1)
				throw("mpz_init_set_str");
		}

		// 5.4 Conversion Functions

		explicit operator unsigned long int() const {
			return mpz_get_ui(m_data);
		}

		explicit operator signed long int() const {
			return mpz_get_si(m_data);
		}

		explicit operator double() const {
			return mpz_get_d(m_data);
		}

		// stolen from boost.multiprecision
		std::string str(std::ios_base::fmtflags f = std::ios_base::fmtflags(0)) const
		{
			int base = 10;
			if((f & std::ios_base::oct) == std::ios_base::oct)
				base = 8;
			else if((f & std::ios_base::hex) == std::ios_base::hex)
				base = 16;
			//
			// sanity check, bases 8 and 16 are only available for positive numbers:
			//
			if((base != 10) && (mpz_sgn(m_data) < 0))
				BOOST_THROW_EXCEPTION(std::runtime_error("Formatted output in bases 8 or 16 is only available for positive numbers"));
			void *(*alloc_func_ptr) (size_t);
			void *(*realloc_func_ptr) (void *, size_t, size_t);
			void (*free_func_ptr) (void *, size_t);
			const char* ps = mpz_get_str (0, base, m_data);
			std::string s = ps;
			mp_get_memory_functions(&alloc_func_ptr, &realloc_func_ptr, &free_func_ptr);
			(*free_func_ptr)((void*)ps, std::strlen(ps) + 1);

			if((base != 10) && (f & std::ios_base::showbase))
			{
				int pos = s[0] == '-' ? 1 : 0;
				const char* pp = base == 8 ? "0" : "0x";
				s.insert(pos, pp);
			}
			if((f & std::ios_base::showpos) && (s[0] != '-'))
				s.insert(0, 1, '+');

			return s;
		}

		// 5.5 Arithmetic Functions

		mpz_int operator+(mpz_int const& rhs) const {
			mpz_int res;
			mpz_add(res.m_data, m_data, rhs.m_data);
			return res;
		}

		mpz_int& operator+=(mpz_int const& rhs) {
			mpz_add(m_data, m_data, rhs.m_data);
			return *this;
		}

		mpz_int operator+(unsigned long int rhs) const {
			mpz_int res;
			mpz_add_ui(res.m_data, m_data, rhs);
			return res;
		}

		mpz_int& operator+=(unsigned long int rhs) {
			mpz_add_ui(m_data, m_data, rhs);
			return *this;
		}

		mpz_int operator-(mpz_int const& rhs) const {
			mpz_int res;
			mpz_sub(res.m_data, m_data, rhs.m_data);
			return res;
		}

		mpz_int& operator-=(mpz_int const& rhs) {
			mpz_sub(m_data, m_data, rhs.m_data);
			return *this;
		}

		mpz_int operator-(unsigned long int rhs) const {
			mpz_int res;
			mpz_sub_ui(res.m_data, m_data, rhs);
			return res;
		}

		mpz_int& operator-=(unsigned long int rhs) {
			mpz_sub_ui(m_data, m_data, rhs);
			return *this;
		}

		mpz_int operator*(mpz_int const& rhs) const {
			mpz_int res;
			mpz_mul(res.m_data, m_data, rhs.m_data);
			return res;
		}

		mpz_int& operator*=(mpz_int const& rhs) {
			mpz_mul(m_data, m_data, rhs.m_data);
			return *this;
		}

		mpz_int operator-() const {
			mpz_int res;
			mpz_neg(res.m_data, m_data);
			return res;
		}

		mpz_int operator<<(mp_bitcnt_t rhs) const {
			mpz_int res;
			mpz_mul_2exp(res.m_data, m_data, rhs);
			return res;
		}

		mpz_int& operator<<=(mp_bitcnt_t rhs) {
			mpz_mul_2exp(m_data, m_data, rhs);
			return *this;
		}

		mpz_int operator>>(mp_bitcnt_t rhs) const {
			mpz_int res;
			mpz_fdiv_q_2exp(res.m_data, m_data, rhs);
			return res;
		}

		mpz_int& operator>>=(mp_bitcnt_t rhs) {
			mpz_fdiv_q_2exp(m_data, m_data, rhs);
			return *this;
		}
	
		// 5.6 Division Functions

		mpz_int operator/(mpz_int const& rhs) {
			mpz_int res;
			mpz_tdiv_q(res.m_data, m_data, rhs.m_data);
			return res;
		}

		mpz_int& operator/=(mpz_int const& rhs) {
			mpz_tdiv_q(m_data, m_data, rhs.m_data);
			return *this;
		}

		mpz_int operator/(unsigned long int rhs) {
			mpz_int res;
			mpz_tdiv_q_ui(res.m_data, m_data, rhs);
			return res;
		}

		mpz_int& operator/=(unsigned long int rhs) {
			mpz_tdiv_q_ui(m_data, m_data, rhs);
			return *this;
		}

		mpz_int operator%(mpz_int const& rhs) {
			mpz_int res;
			mpz_mod(res.m_data, m_data, rhs.m_data);
			return res;
		}

		mpz_int& operator%=(mpz_int const& rhs) {
			mpz_mod(m_data, m_data, rhs.m_data);
			return *this;
		}

		mpz_int operator%(unsigned long int rhs) {
			mpz_int res;
			mpz_mod_ui(res.m_data, m_data, rhs);
			return res;
		}

		mpz_int& operator%=(unsigned long int rhs) {
			mpz_mod_ui(m_data, m_data, rhs);
			return *this;
		}

		// 5.10 Comparison Functions

		bool operator<(mpz_int const& rhs) const {
			return mpz_cmp(m_data, rhs.m_data) < 0;
		}

		bool operator==(mpz_int const& rhs) const {
			return mpz_cmp(m_data, rhs.m_data) == 0;
		}

		bool operator!=(mpz_int const& rhs) const {
			return mpz_cmp(m_data, rhs.m_data) != 0;
		}

		bool operator>(mpz_int const& rhs) const {
			return mpz_cmp(m_data, rhs.m_data) > 0;
		}

		// 5.11 Logical and Bit Manipulation Functions

		// TODO later

		// friend functions

		friend mpz_int modinv(mpz_int const& a, mpz_int const& b)
		{
			mpz_int r;
			mpz_invert(r.m_data, a.m_data, b.m_data);
			return r;
		}
	};

	// stolen from boost.multiprecision
	std::ostream& operator<<(std::ostream& o, mpz_int const& rhs)
	{
		std::string s = rhs.str(o.flags());
		std::streamsize ss = o.width();
		if(ss > static_cast<std::streamsize>(s.size()))
		{
			char fill = o.fill();
			if((o.flags() & std::ios_base::left) == std::ios_base::left)
				s.append(static_cast<std::string::size_type>(ss - s.size()), fill);
			else
				s.insert(0, static_cast<std::string::size_type>(ss - s.size()), fill);
		}
		return o << s;
	}
}

#endif // GMP_HPP
