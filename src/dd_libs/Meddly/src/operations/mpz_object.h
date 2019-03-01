

// $Id$

/*
    Meddly: Multi-terminal and Edge-valued Decision Diagram LibrarY.
    Copyright (C) 2009, Iowa State University Research Foundation, Inc.

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published 
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MPZ_OBJECT
#define MPZ_OBJECT

#ifdef HAVE_LIBGMP
#include <gmp.h>

namespace MEDDLY {

class mpz_object : public ct_object {
  mpz_t value;
public:
  mpz_object();
  mpz_object(const mpz_t &v);
  mpz_object(const mpz_object &v);
  virtual ~mpz_object();
  virtual opnd_type getType();
  
  inline void copyInto(mpz_t &x) const {
    mpz_set(x, value);
  }
  inline void copyInto(mpz_object &x) const {
    mpz_set(x.value, value);
  }
  inline void setValue(long i) {
    mpz_set_si(value, i);
  }
  void show(output &strm) const;
  inline void multiply(long i) {
    mpz_mul_si(value, value, i);
  }
  inline void add(const mpz_object &x) {
    mpz_add(value, value, x.value);
  }

  static void initBuffer();
  static void clearBuffer();

private:
  
  static char* buffer;  // for show().
  static int bufsize;   // for show().
  static void enlargeBuffer(int digits);
};

}

#endif
#endif

