
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



// TODO: Testing

#include "defines.h"

#include <cstdio>
#include <iostream>

// ******************************************************************
// *                                                                *
// *                         input  methods                         *
// *                                                                *
// ******************************************************************

MEDDLY::input::input()
{
}

MEDDLY::input::~input()
{
}

void MEDDLY::input::stripWS()
{
  bool comment = false;
  for (;;) {
    int c = get_char();
    if (EOF == c) throw error(error::INVALID_FILE);
    if ('\n'== c) {
      comment = false;
      continue;
    }
    if (comment) continue;
    if (' ' == c) continue;
    if ('\t' == c) continue;
    if ('\r' == c) continue;
    if ('#' == c) {
      comment = true;
      continue;
    }
    // not whitespace
    unget(c);
    return;
  }
}

void MEDDLY::input::consumeKeyword(const char* keyword)
{
    for ( ; *keyword; keyword++) {
      int c = get_char();
      if (c != *keyword) throw error(error::INVALID_FILE);
    }
}

// ******************************************************************
// *                                                                *
// *                       FILE_input methods                       *
// *                                                                *
// ******************************************************************

MEDDLY::FILE_input::FILE_input(FILE* _inf)
{
  inf = _inf;
}

MEDDLY::FILE_input::~FILE_input()
{
  // DO NOT close file, user may want it!
}

bool MEDDLY::FILE_input::eof() const
{
  return feof(inf);
}

int MEDDLY::FILE_input::get_char()
{
  // TBD - deal with errors
  return fgetc(inf);
}

void MEDDLY::FILE_input::unget(char x)
{
  if (EOF==ungetc(x, inf)) {
    throw error(error::COULDNT_READ); 
    // probably not the most descriptive error message,
    // but it was in fact an error on file input
  }
}

long MEDDLY::FILE_input::get_integer()
{
  long data;
  if (fscanf(inf, "%ld", &data) != 1) {
    throw error(error::INVALID_FILE);
  }
  return data;
}

double MEDDLY::FILE_input::get_real()
{
  double data;
  if (fscanf(inf, "%lf", &data) != 1) {
    throw error(error::INVALID_FILE);
  }
  return data;
}

int MEDDLY::FILE_input::read(int bytes, unsigned char* buffer)
{
  return fread(buffer, 1, bytes, inf);
}

// ******************************************************************
// *                                                                *
// *                     istream_input  methods                     *
// *                                                                *
// ******************************************************************

MEDDLY::istream_input::istream_input(std::istream &inf) : in(inf)
{
}

MEDDLY::istream_input::~istream_input()
{
  // DO NOT close file, user may want it!
}

bool MEDDLY::istream_input::eof() const
{
  return in.eof();
}

int MEDDLY::istream_input::get_char()
{
  return in.get();
  // TBD - how to deal with errors?
}

void MEDDLY::istream_input::unget(char x)
{
  in.unget();
}

long MEDDLY::istream_input::get_integer()
{
  long data;
  in >> data;
  // TBD - how to deal with errors?
  return data;
}

double MEDDLY::istream_input::get_real()
{
  double data;
  in >> data;
  // TBD - how to deal with errors?
  return data;
}

int MEDDLY::istream_input::read(int bytes, unsigned char* buffer)
{
  in.read((char*) buffer, bytes);
  return in.gcount();
}


// ******************************************************************
// *                                                                *
// *                         output methods                         *
// *                                                                *
// ******************************************************************

MEDDLY::output::output()
{
}

MEDDLY::output::~output()
{
}

void MEDDLY::output::put_mem(long m, bool human)
{
  if ((!human) || (m<1024)) {
    put(m);
    put(" bytes");
    return;
  }
  double approx = m;
  approx /= 1024;
  if (approx < 1024) {
    put(approx, 3, 2, 'f');
    put(" Kbytes");
    return;
  }
  approx /= 1024;
  if (approx < 1024) {
    put(approx, 3, 2, 'f');
    put(" Mbytes");
    return;
  }
  approx /= 1024;
  if (approx < 1024) {
    put(approx, 3, 2, 'f');
    put(" Gbytes");
    return;
  }
  approx /= 1024;
  put(approx, 3, 2, 'f');
  put(" Tbytes");
}

// ******************************************************************
// *                                                                *
// *                      FILE_output  methods                      *
// *                                                                *
// ******************************************************************

MEDDLY::FILE_output::FILE_output(FILE* _outf)
{
  outf = _outf;
}

MEDDLY::FILE_output::~FILE_output()
{
  // DON'T close the file, the user might need to add more to it
}

void MEDDLY::FILE_output::put(char x)
{
  if (EOF == fputc(x, outf)) {
    throw error(error::COULDNT_WRITE);
  }
}

void MEDDLY::FILE_output::put(const char* x, int w)
{
  if (fprintf(outf, "%*s", w, x)<1) {
    throw error(error::COULDNT_WRITE);
  }
}

void MEDDLY::FILE_output::put(long x, int w)
{
  if (fprintf(outf, "%*ld", w, x)<1) {
    throw error(error::COULDNT_WRITE);
  }
}

void MEDDLY::FILE_output::put_hex(unsigned long x, int w)
{
  if (fprintf(outf, "%*lx", w, x)<1) {
    throw error(error::COULDNT_WRITE);
  }
}


void MEDDLY::FILE_output::put(double x, int w, int p, char f)
{
  int out=-1;
  switch (f) {
    case 'e':
              out = fprintf(outf, "%*.*e", w, p, x);
              break;
    case 'f':
              out = fprintf(outf, "%*.*f", w, p, x);
              break;
    default:  // 'g' is the default
              out = fprintf(outf, "%*.*g", w, p, x);
              break;
  }
  if (out < 1) {
    throw error(error::COULDNT_WRITE);
  }
}

int MEDDLY::FILE_output::write(int bytes, const unsigned char* buffer)
{
  return fwrite(buffer, 1, bytes, outf);
}

void MEDDLY::FILE_output::flush()
{
  fflush(outf);
}

// ******************************************************************
// *                                                                *
// *                     ostream_output methods                     *
// *                                                                *
// ******************************************************************

MEDDLY::ostream_output::ostream_output(std::ostream &outs) : out(outs)
{
}

MEDDLY::ostream_output::~ostream_output()
{
  // DON'T close the file, the user might need to add more to it
}

void MEDDLY::ostream_output::put(char x)
{
  out.put(x);
  // TBD - what about errors?
}

void MEDDLY::ostream_output::put(const char* x, int w)
{
  out.width(w);
  out << x;
  // TBD - what about errors?
}

void MEDDLY::ostream_output::put(long x, int w)
{
  out.setf(std::ios::dec, std::ios::basefield);
  out.width(w);
  out << x;
  // TBD - what about errors?
}

void MEDDLY::ostream_output::put_hex(unsigned long x, int w)
{
  out.setf(std::ios::hex, std::ios::basefield);
  out.width(w);
  out << x;
  // TBD - what about errors?
}

void MEDDLY::ostream_output::put(double x, int w, int p, char f)
{
  switch (f) {
    case 'e':
              out.setf(std::ios_base::scientific, std::ios_base::floatfield);
              break;

    case 'f':
              out.setf(std::ios_base::fixed, std::ios_base::floatfield);
              break;

    default:  // g is the default
              out.setf(std::ios_base::fmtflags(0), std::ios_base::floatfield);
              break;
  }
  out.setf(std::ios::dec, std::ios::basefield);
  out.precision(p);
  out.width(w);
  out << x;
}

int MEDDLY::ostream_output::write(int bytes, const unsigned char* buffer)
{
  out.write((const char*) buffer, bytes);
  // Not sure how to catch this one, so...
  return bytes;
}

void MEDDLY::ostream_output::flush()
{
  out.flush();
}

