
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


#ifndef BYTEPACK_H
#define BYTEPACK_H

// ******************************************************************
// *                                                                *
// *                                                                *
// *     Utilities to compact integers  into fewer bits by size     *
// *                                                                *
// *                                                                *
// ******************************************************************

// ******************************************************************
// *                                                                *
// *                  Determine space requirements                  *
// *                                                                *
// ******************************************************************

template <class INT>
inline int bytesRequired4(INT a)
{
  // proceed with byte examinations
  static const unsigned long byte2 = 0xff00;
  static const unsigned long byte3 = byte2<<8;
  static const unsigned long byte4 = byte3<<8;
  if (a & (byte4 | byte3)) {
    if (a & byte4) {
      return 4;
    } else {
      return 3;
    }
  } else {
    if (a & byte2) {
      return 2;
    } else {
      return 1;
    }
  }
}

template <class INT>
inline int bytesRequired8(INT a)
{
  // proceed with byte examinations
  static const unsigned long byte2 = 0xff00;
  static const unsigned long byte3 = byte2<<8;
  static const unsigned long byte4 = byte3<<8;
  static const unsigned long byte5 = byte4<<8;
  static const unsigned long byte6 = byte5<<8;
  static const unsigned long byte7 = byte6<<8;
  static const unsigned long byte8 = byte7<<8;
  if (a & (byte8 | byte7 | byte6 | byte5)) {
    if (a & (byte8 | byte7)) {
      if (a & byte8) {
        return 8;
      } else {
        return 7;
      }
    } else {
      if (a & byte6) {
        return 6;
      } else {
        return 5;
      }
    }
  } else {
    if (a & (byte4 | byte3)) {
      if (a & byte4) {
        return 4;
      } else {
        return 3;
      }
    } else {
      if (a & byte2) {
        return 2;
      } else {
        return 1;
      }
    }
  }
}

template <int N, class INT>
inline int bytesRequired(INT a);

template <>
inline int bytesRequired<4>(int a)
{
  return bytesRequired4(a);
}

template <>
inline int bytesRequired<4>(long a)
{
  return bytesRequired4(a);
}

template <>
inline int bytesRequired<8>(long a)
{
  return bytesRequired8(a);
}


template <class INT>
inline void stripDownEncodingForSizing(INT &a)
{
  if (a<0) {
    // terminal value
    a <<= 1;
    if (a<0) {
      a <<= 1;
      a = -a;
    } else {
      a <<= 1;
    }
  } else {
    // nonterminal node
    a <<= 1;
  }
}

template <class INT>
inline void stripSignedEncodingForSizing(INT &a)
{
  a = (a<0) ? -(a<<1) : a<<1; 
}

/**
      Determine the bytes required for downward pointer \a a.

      Downward pointers follow the rules:
        if positive, it points to a node
        if negative, it encodes a terminal node
        
      so the implementation takes that encoding into account.
*/
inline int bytesRequiredForDown(int a)
{
  stripDownEncodingForSizing(a);
  return bytesRequired<sizeof(int)>(a);
}

/**
      Determine the bytes required for downward pointer \a a.

      Downward pointers follow the rules:
        if positive, it points to a node
        if negative, it encodes a terminal node
        
      so the implementation takes that encoding into account.
*/
inline int bytesRequiredForDown(long a)
{
  stripDownEncodingForSizing(a);
  return bytesRequired<sizeof(long)>(a);
}

/**
      Determine the bytes required for a signed value \a a.
      We are effectively assuming 2s complement storage.
*/
inline int bytesRequiredForSigned(int a)
{
  stripSignedEncodingForSizing(a);
  return bytesRequired<sizeof(int)>(a);
}

/**
      Determine the bytes required for a signed value \a a.
      We are effectively assuming 2s complement storage.
*/
inline int bytesRequiredForSigned(long a)
{
  stripSignedEncodingForSizing(a);
  return bytesRequired<sizeof(long)>(a);
}


// ******************************************************************
// *                                                                *
// *       Convert from natural  to smaller space requirement       *
// *                                                                *
// ******************************************************************

template <int bytes, class INT>
inline void rawToData(INT a, unsigned char* b)
{
  MEDDLY_DCASSERT(bytes <= sizeof(INT));
  //
  // The compiler is smart enough to optimize out these if's
  //
  b[0] = a & 0xff;
  if (bytes > 1)  { a >>= 8;  b[1] = a & 0xff;  }
  if (bytes > 2)  { a >>= 8;  b[2] = a & 0xff;  }
  if (bytes > 3)  { a >>= 8;  b[3] = a & 0xff;  }
  if (bytes > 4)  { a >>= 8;  b[4] = a & 0xff;  }
  if (bytes > 5)  { a >>= 8;  b[5] = a & 0xff;  }
  if (bytes > 6)  { a >>= 8;  b[6] = a & 0xff;  }
  if (bytes > 7)  { a >>= 8;  b[7] = a & 0xff;  }
}

template <int bytes, class INT>
inline void signedToData(INT L, unsigned char* d)
{
  MEDDLY_DCASSERT(bytes <= sizeof(INT));
  rawToData<bytes>(L, d);
}

template <int bytes, class INT>
inline void downToData(INT  P, unsigned char* d)
{
  MEDDLY_DCASSERT(bytes <= sizeof(INT));
  // positive P: as usual.
  if (P >= 0) {
    rawToData<bytes>(P, d);
    return;
  }

  // negative P: this is a terminal pointer.
  //              next msb set - terminal value is negative.
  //
  //  No conversion necessary because msb propogates when we shift
  static const unsigned long nmsb = (0x40UL) << ((sizeof(INT)-1)*8);
  if (P & nmsb) {
    rawToData<bytes>(P, d);
    return;
  }

  // negative P: this is a terminal pointer.
  //              next msb clr - terminal value is positive.
  //              
  //  The thing to do here is deal with the msb manually:
  //  clear msb, encode, set msb.
  static const unsigned long msboff = ~ ((0x80UL) << ((sizeof(INT)-1)*8));
  rawToData<bytes>(P & msboff, d);
  d[bytes-1] |= 0x80;
}


// ******************************************************************
// *                                                                *
// *       Convert from smaller  to natural space requirement       *
// *                                                                *
// ******************************************************************

template <int bytes, class INT>
inline void dataToRaw(const unsigned char* b, INT &a)
{
  MEDDLY_DCASSERT(bytes <= sizeof(INT));
  //
  // The compiler is smart enough to optimize out these if's
  //
  if (bytes > 7)  { a |= b[7];  a <<= 8;  }
  if (bytes > 6)  { a |= b[6];  a <<= 8;  }
  if (bytes > 5)  { a |= b[5];  a <<= 8;  }
  if (bytes > 4)  { a |= b[4];  a <<= 8;  }
  if (bytes > 3)  { a |= b[3];  a <<= 8;  }
  if (bytes > 2)  { a |= b[2];  a <<= 8;  }
  if (bytes > 1)  { a |= b[1];  a <<= 8;  }
  a |= b[0];
}

template <class INT>
inline void dataToRaw(const unsigned char* d, int bytes, INT& L)
{
  switch (bytes) {
    case 1:   dataToRaw<1>(d, L);     return;
    case 2:   dataToRaw<2>(d, L);     return;
    case 3:   dataToRaw<3>(d, L);     return;
    case 4:   dataToRaw<4>(d, L);     return;
    case 5:   dataToRaw<5>(d, L);     return;
    case 6:   dataToRaw<6>(d, L);     return;
    case 7:   dataToRaw<7>(d, L);     return;
    case 8:   dataToRaw<8>(d, L);     return;

    default:
      MEDDLY_DCASSERT(0);
  }
}

template <int bytes, class INT>
inline void dataToSigned(const unsigned char* d, INT& L)
{
  MEDDLY_DCASSERT(bytes <= sizeof(INT));
  // deal with negatives properly
  if (d[bytes-1] & 0x80) {
    L = INT((~0UL) << 8);
  } else {
    L = 0;
  }
  dataToRaw<bytes>(d, L);
}

template <class INT>
inline void dataToSigned(const unsigned char* d, int bytes, INT& L)
{
  switch (bytes) {
    case 1:   dataToSigned<1>(d, L);    return;
    case 2:   dataToSigned<2>(d, L);    return;
    case 3:   dataToSigned<3>(d, L);    return;
    case 4:   dataToSigned<4>(d, L);    return;
    case 5:   dataToSigned<5>(d, L);    return;
    case 6:   dataToSigned<6>(d, L);    return;
    case 7:   dataToSigned<7>(d, L);    return;
    case 8:   dataToSigned<8>(d, L);    return;

    default:
      MEDDLY_DCASSERT(0);
  }
}

template <int bytes, class INT>
inline void dataToUnsigned(const unsigned char* b, INT &a)
{
  a = 0;
  dataToRaw<bytes>(b, a);
}

template <class INT>
inline void dataToUnsigned(const unsigned char* b, int bytes, INT &a)
{
  a = 0;
  dataToRaw(b, bytes, a);
}

//
// This business is annoying but
// it prevents several compiler warnings about
// "left shift count >= width of type".
// Since we set this up, might as well use it nicely.
//
template <int bytes, int sizeofint, class INT>
inline void moveMSB(INT& P);

// These can't ever happen

template <> inline void moveMSB<5, 4>(int& P)   { MEDDLY_DCASSERT(0); }
template <> inline void moveMSB<6, 4>(int& P)   { MEDDLY_DCASSERT(0); }
template <> inline void moveMSB<7, 4>(int& P)   { MEDDLY_DCASSERT(0); }
template <> inline void moveMSB<8, 4>(int& P)   { MEDDLY_DCASSERT(0); }

// These can't ever happen

template <> inline void moveMSB<5, 4>(long& P)  { MEDDLY_DCASSERT(0); }
template <> inline void moveMSB<6, 4>(long& P)  { MEDDLY_DCASSERT(0); }
template <> inline void moveMSB<7, 4>(long& P)  { MEDDLY_DCASSERT(0); }
template <> inline void moveMSB<8, 4>(long& P)  { MEDDLY_DCASSERT(0); }

// These are no-ops

template <> inline void moveMSB<4, 4>(int& P) { 
  MEDDLY_DCASSERT(sizeof(int) == 4); // sanity check
}

template <> inline void moveMSB<4, 4>(long& P) { 
  MEDDLY_DCASSERT(sizeof(long) == 4); // sanity check
}

template <> inline void moveMSB<8, 8>(long& P) {
  MEDDLY_DCASSERT(sizeof(long) == 8); // sanity check
}

// Everything else

template <int bytes, int sizeofint, class INT>
inline void moveMSB(INT& P)
{
  MEDDLY_DCASSERT(sizeof(INT) == sizeofint);
  // if (bytes < sizeofint) {
    P = (P & ~(0x80UL << ((bytes-1)*8)) )      // old msb off
        | ((0x80UL) << ((sizeofint-1)*8));     // new msb on
  // }
}

template <int bytes, int sizeofint, class INT>
inline void dataToDown(const unsigned char* d, INT& P)
{
  MEDDLY_DCASSERT(sizeofint == sizeof(INT));
  MEDDLY_DCASSERT(bytes <= sizeofint);
  // Is this a terminal value?
  if (d[bytes-1] & 0x80) {
    // YES.
    // Is this a negative terminal value?
    if (d[bytes-1] & 0x40) {
      // YES.
      // Easy case: same as ordinary negatives.
      P = INT((~0UL) << 8);
      dataToRaw<bytes>(d, P);
      return;
    }
    // NO.
    // Positive terminal value.
    P = 0;
    dataToRaw<bytes>(d, P);

    // Move MSB if necessary
    moveMSB<bytes, sizeofint>(P);

    return;
  }

  // non-terminal value: as usual
  P = 0;
  dataToRaw<bytes>(d, P);
}

template <int bytes, class INT>
inline void dataToDown(const unsigned char* d, INT& P)
{
  dataToDown<bytes, sizeof(INT)>(d, P);
}

template <class INT>
inline void dataToDown(const unsigned char* d, int bytes, INT& L)
{
  switch (bytes) {
    case 1:   dataToDown<1>(d, L);    return;
    case 2:   dataToDown<2>(d, L);    return;
    case 3:   dataToDown<3>(d, L);    return;
    case 4:   dataToDown<4>(d, L);    return;
    case 5:   dataToDown<5>(d, L);    return;
    case 6:   dataToDown<6>(d, L);    return;
    case 7:   dataToDown<7>(d, L);    return;
    case 8:   dataToDown<8>(d, L);    return;

    default:
      MEDDLY_DCASSERT(0);
  }
}



#endif  // include guard

