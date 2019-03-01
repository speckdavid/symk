
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

#ifndef HASH_STREAM_H
#define HASH_STREAM_H

#include "meddly_expert.h"

namespace MEDDLY {
  class hash_stream;
};

// #define DEBUG_HASH

/**
    Class to hash a stream of unsigned integers.
    Length of the stream can be unknown.
    Based on Bob Jenkin's Hash; see
    http://burtleburtle.net/bob/hash/doobs.html
*/
class MEDDLY::hash_stream {
    unsigned z[3];
    int slot;
  public:
    hash_stream() { }
  protected:
    inline static unsigned rot(unsigned x, int k) { 
      return (((x)<<(k)) | ((x)>>(32-(k))));
    }
    inline static void mix(unsigned &a, unsigned &b, unsigned &c) {
        a -= c;  a ^= rot(c, 4);  c += b; 
        b -= a;  b ^= rot(a, 6);  a += c;
        c -= b;  c ^= rot(b, 8);  b += a;
        a -= c;  a ^= rot(c,16);  c += b;
        b -= a;  b ^= rot(a,19);  a += c;
        c -= b;  c ^= rot(b, 4);  b += a;
    }
    inline static void final(unsigned &a, unsigned &b, unsigned &c) {
        c ^= b; c -= rot(b,14);
        a ^= c; a -= rot(c,11);
        b ^= a; b -= rot(a,25);
        c ^= b; c -= rot(b,16);
        a ^= c; a -= rot(c,4); 
        b ^= a; b -= rot(a,14);
        c ^= b; c -= rot(b,24);
    }
    inline void mix()   { mix(z[2], z[1], z[0]); }
    inline void final() { final(z[2], z[1], z[0]); }
  public:
    inline void start(unsigned init) {
#ifdef DEBUG_HASH
        printf("hash_stream::start %u\n", init);
#endif
        z[2] = init;
        z[1] = 0;
        z[0] = 0xdeadbeef;
        slot = 2;
    }
    inline unsigned finish() {
        final();
#ifdef DEBUG_HASH
        printf("hash_stream::finish: %u\n", z[0]);
#endif
        return z[0];
    }
    inline void push(unsigned v) {
#ifdef DEBUG_HASH
        printf("    push %u\n", v);
#endif
        if (slot) {
          slot--;
          z[slot] += v;
        } else {
          mix();
          z[2] += v;
          slot = 2;
        }
    }
    inline void push(unsigned v1, unsigned v2) {
#ifdef DEBUG_HASH
        printf("    push %u, %u\n", v1, v2);
#endif
        switch (slot) {
            case 0: 
                mix();  
                z[2] += v1;
                z[1] += v2;
                slot = 1;
                return;

            case 1: 
                z[0] += v1;
                mix();
                z[2] += v2;
                slot = 2;
                return;

            case 2: 
                z[1] += v1;
                z[0] += v2;
                slot = 0;
                return;

            default: throw error(error::MISCELLANEOUS);
        };
    }
    inline void push(unsigned v1, unsigned v2, unsigned v3) {
#ifdef DEBUG_HASH
        printf("    push %u, %u, %u\n", v1, v2, v3);
#endif
        switch (slot) {
            case 0: 
                mix();  
                z[2] += v1;
                z[1] += v2;
                z[0] += v3; 
                return;

            case 1: 
                z[0] += v1;
                mix();
                z[2] += v2;
                z[1] += v3;
                return;

            case 2: 
                z[1] += v1;
                z[0] += v2;
                mix(); 
                z[2] += v3;
                return;

            default: throw error(error::MISCELLANEOUS);
        };
    }
    inline void push(const void* data, size_t bytes) {
      const unsigned* hack = (const unsigned*) data;
      size_t num_unsigneds = (bytes / sizeof(unsigned));
      const unsigned* hackend = hack + num_unsigneds;
      for (; hack < hackend; hack++) {
        push(*hack);
      }
      const unsigned char* lastfew = (const unsigned char*) hackend;
      unsigned leftover;
      switch (bytes % sizeof(unsigned)) {
        case 0:
            return;

        case 1:
            push(lastfew[0]);
            return;

        case 2:
            leftover = lastfew[0];
            leftover <<= 8;
            leftover |= lastfew[1];
            push(leftover);
            return;

        case 3:
            leftover = lastfew[0];
            leftover <<= 8;
            leftover |= lastfew[1];
            leftover <<= 8;
            leftover |= lastfew[2];
            push(leftover);
            return;

        // how large can an unsigned be?
        // just to be sure:

        case 4:
            leftover = lastfew[0];
            leftover <<= 8;
            leftover |= lastfew[1];
            leftover <<= 8;
            leftover |= lastfew[2];
            leftover <<= 8;
            leftover |= lastfew[3];
            push(leftover);
            return;

        case 5:
            leftover = lastfew[0];
            leftover <<= 8;
            leftover |= lastfew[1];
            leftover <<= 8;
            leftover |= lastfew[2];
            leftover <<= 8;
            leftover |= lastfew[3];
            leftover <<= 8;
            leftover |= lastfew[4];
            push(leftover);
            return;

        case 6:
            leftover = lastfew[0];
            leftover <<= 8;
            leftover |= lastfew[1];
            leftover <<= 8;
            leftover |= lastfew[2];
            leftover <<= 8;
            leftover |= lastfew[3];
            leftover <<= 8;
            leftover |= lastfew[4];
            leftover <<= 8;
            leftover |= lastfew[5];
            push(leftover);
            return;

        case 7:
            leftover = lastfew[0];
            leftover <<= 8;
            leftover |= lastfew[1];
            leftover <<= 8;
            leftover |= lastfew[2];
            leftover <<= 8;
            leftover |= lastfew[3];
            leftover <<= 8;
            leftover |= lastfew[4];
            leftover <<= 8;
            leftover |= lastfew[5];
            leftover <<= 8;
            leftover |= lastfew[6];
            push(leftover);
            return;

        default:
            throw error(error::MISCELLANEOUS);
      }
    }
}; 

#endif
