
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBGMP 

#include <gmp.h>
#include "../defines.h"
#include "mpz_object.h"


// ******************************************************************
// *                                                                *
// *                       mpz_object methods                       *
// *                                                                *
// ******************************************************************

MEDDLY::mpz_object::mpz_object()
{
  mpz_init(value);
}

MEDDLY::mpz_object::mpz_object(const mpz_t &v)
{
  mpz_init_set(value, v);
}

MEDDLY::mpz_object::mpz_object(const mpz_object &x)
{
  mpz_init_set(value, x.value);
}


MEDDLY::mpz_object::~mpz_object()
{
  mpz_clear(value);
}

MEDDLY::opnd_type MEDDLY::mpz_object::getType()
{
  return HUGEINT;
}

void MEDDLY::mpz_object::show(output &strm) const
{
  // Make sure temporary string has enough space
  int digits = mpz_sizeinbase(value, 10)+2;
  enlargeBuffer(digits);
  // Write into a string
  mpz_get_str(buffer, 10, value);
  // Display the string
  strm.put(buffer);
}

void MEDDLY::mpz_object::initBuffer()
{
  buffer = 0;
  bufsize = 0;
}

void MEDDLY::mpz_object::clearBuffer()
{
  free(buffer);
  buffer = 0;
  bufsize = 0;
}

void MEDDLY::mpz_object::enlargeBuffer(int digits)
{
  if (digits > bufsize) {
    int newbufsize = (1+digits / 1024) * 1024;
    if (newbufsize <= 0) throw error(error::MISCELLANEOUS);
    char* newbuf = (char*) realloc(buffer, newbufsize);
    if (0==newbuf) throw error(error::INSUFFICIENT_MEMORY);
    buffer = newbuf;
    bufsize = newbufsize;
  }
}

char* MEDDLY::mpz_object::buffer;
int   MEDDLY::mpz_object::bufsize;



// ******************************************************************
// *                                                                *
// *                      mpz_object functions                      *
// *                                                                *
// ******************************************************************


MEDDLY::ct_object& MEDDLY::get_mpz_wrapper()
{
  static MEDDLY::mpz_object foo;
  return foo;
}

void MEDDLY::unwrap(const ct_object &x, mpz_t &value)
{
  using namespace MEDDLY;
  const mpz_object &mx = static_cast <const mpz_object &> (x);
  mx.copyInto(value);
}

#endif

