
// $Id$

/*
    Meddly: Multi-terminal and Edge-valued Decision Diagram LibrarY.
    Copyright (C) 2011, Iowa State University Research Foundation, Inc.

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

#include <cstdio>
#include <cstdlib>

#include "../src/timer.h"

// super hack

#include "../src/storage/bytepack.h"

// now, test those functions :^)

unsigned char buffer[10];

const int stop = 2147483647; 
// const int stop = 134217728;
// const int stop = 16777216;
// const int stop = 1048576;

void encodeInt(int a, int bytes)
{
  switch (bytes) {
    case 1:   signedToData<1>(a, buffer);  return;
    case 2:   signedToData<2>(a, buffer);  return;
    case 3:   signedToData<3>(a, buffer);  return;
    case 4:   signedToData<4>(a, buffer);  return;
    case 5:   signedToData<5>(a, buffer);  return;
    case 6:   signedToData<6>(a, buffer);  return;
    case 7:   signedToData<7>(a, buffer);  return;
    case 8:   signedToData<8>(a, buffer);  return;

    default:
      fprintf(stderr, "Bad number of bytes for encoding: %d\n", bytes);
      exit(1);
  }
}

void decodeInt(int &a, int bytes)
{
  return dataToSigned(buffer, bytes, a);
}

void encodeLong(long a, int bytes)
{
  switch (bytes) {
    case 1:   signedToData<1>(a, buffer);  return;
    case 2:   signedToData<2>(a, buffer);  return;
    case 3:   signedToData<3>(a, buffer);  return;
    case 4:   signedToData<4>(a, buffer);  return;
    case 5:   signedToData<5>(a, buffer);  return;
    case 6:   signedToData<6>(a, buffer);  return;
    case 7:   signedToData<7>(a, buffer);  return;
    case 8:   signedToData<8>(a, buffer);  return;

    default:
      fprintf(stderr, "Bad number of bytes for encoding: %d\n", bytes);
      exit(1);
  }
}

void decodeLong(long &a, int bytes)
{
  switch (bytes) {
    case 1:   dataToSigned<1>(buffer, a);  return;
    case 2:   dataToSigned<2>(buffer, a);  return;
    case 3:   dataToSigned<3>(buffer, a);  return;
    case 4:   dataToSigned<4>(buffer, a);  return;
    case 5:   dataToSigned<5>(buffer, a);  return;
    case 6:   dataToSigned<6>(buffer, a);  return;
    case 7:   dataToSigned<7>(buffer, a);  return;
    case 8:   dataToSigned<8>(buffer, a);  return;

    default:
      fprintf(stderr, "Bad number of bytes for encoding: %d\n", bytes);
      exit(1);
  }
}

void encodeDown(long a, int bytes)
{
  switch (bytes) {
    case 1:   downToData<1>(a, buffer);  return;
    case 2:   downToData<2>(a, buffer);  return;
    case 3:   downToData<3>(a, buffer);  return;
    case 4:   downToData<4>(a, buffer);  return;
    case 5:   downToData<5>(a, buffer);  return;
    case 6:   downToData<6>(a, buffer);  return;
    case 7:   downToData<7>(a, buffer);  return;
    case 8:   downToData<8>(a, buffer);  return;

    default:
      fprintf(stderr, "Bad number of bytes for encoding: %d\n", bytes);
      exit(1);
  }
}

void decodeDown(long &a, int bytes)
{
  return dataToDown(buffer, bytes, a);
}

inline void fprinthex(FILE* s, int i)
{
  for (i--; i>=0; i--) fprintf(s, "%02x ", buffer[i]);
}

void testInt(int a)
{
  int br = bytesRequiredForSigned(a);
  
  for (int b=sizeof(int); b>=br; b--) {
    int x;
    encodeInt(a, b);
    decodeInt(x, b);
    if (a != x) {
      fprintf(stdout, "%d -> ", a);
      fprinthex(stdout, b);
      fprintf(stdout, " -> %d\n", x);
      exit(2);
    }
  }
}

void testLong(long a)
{
  int br = bytesRequiredForSigned(a);
  
  for (int b=sizeof(long); b>=br; b--) {
    long x;
    encodeLong(a, b);
    decodeLong(x, b);
    if (a != x) {
      fprintf(stdout, "%ld -> ", a);
      fprinthex(stdout, b);
      fprintf(stdout, " -> %ld\n", x);
      exit(2);
    }
  }
}

void testDown(long a)
{
  int br = bytesRequiredForDown(a);
  
  for (int b=sizeof(long); b>=br; b--) {
    long x;
    encodeDown(a, b);
    decodeDown(x, b);
    if (a != x) {
      fprintf(stdout, "%ld -> ", a);
      fprinthex(stdout, b);
      fprintf(stdout, " -> %ld\n", x);
      exit(2);
    }
  }
}

int main()
{
  int check, count;
  timer foo;
  printf("Testing integer encodings...\n");
  testInt(0);
  check = 1024;
  count = check;
  for (int i=1; i!=stop; i++) {
    testInt(i);
    testInt(-i);
    count--;
    if (count) continue;
    printf("I %d\n", i);
    if (foo.get_last_interval() < 5000000) {
      if (check < 1073741824) check *= 2;
    }
    count = check;
    foo.note_time();
  }
  printf("Testing long encodings (with shifted ints)...\n");
  testLong(0);
  check = 1024;
  count = check;
  foo.note_time();
  for (int i=1; i!=stop; i++) {
    long L = i;
    while (L << 1) {
      testLong(L);
      testLong(-L);
      L <<= 1;
    }
    count--;
    if (count) continue;
    printf("L %d\n", i);
    if (foo.get_last_interval() < 5000000) {
      if (check < 1073741824) check *= 2;
    }
    count = check;
    foo.note_time();
  }
  printf("Testing down encodings (with shifted ints)...\n");
  testDown(0);
  check = 1024;
  count = check;
  foo.note_time();
  for (int i=1; i!=stop; i++) {
    long L = i;
    while (L << 2) {
      testDown(L);
      testDown(-L);
      static const long msb = (0x80L) << ((sizeof(long)-1)*8);
      testDown(L | msb);
      L <<= 1;
    }
    count--;
    if (count) continue;
    printf("D %d\n", i);
    if (foo.get_last_interval() < 5000000) {
      if (check < 1073741824) check *= 2;
    }
    count = check;
    foo.note_time();
  }
  printf("Done.\n");
}
