
/* $Id$ */

/*
    termview: terminal viewing utility for Meddly trace data.
    Copyright (C) 2015, Iowa State University Research Foundation, Inc.

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "system.h"
#include "parse.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// #define DEBUG_PARSER

// TBD - #include "data.h"

/* ================================================================ 
   |                                                              |
   |                          ignoreLine                          |
   |                                                              |
   ================================================================ */

void ignoreLine(FILE* inf)
{
  for (;;) {
    int c = fgetc(inf);
    if (EOF == c) return;
    if ('\n' == c) return;
  }
}

/* ================================================================ 
   |                                                              |
   |                          matchChar                           |
   |                                                              |
   ================================================================ */

char matchChar(FILE* inf, const char* list)
{
  int c = fgetc(inf);
  if (EOF == c) return 0;
  if ('\n' == c) return 0;
  while (*list) {
    if (*list == c) return c;
    list++;
  }
  /* Not in the list - consume to EOL */
  ignoreLine(inf);
  return 0;
}

/* ================================================================ 
   |                                                              |
   |                          readString                          |
   |                                                              |
   ================================================================ */

int readString(FILE* inf, char* line, int length)
{
  for (int i=0; ; ) {
    int c = fgetc(inf);
    if ('"' == c || '\r' == c || '\n' == c || EOF == c) {
      line[i] = 0;
      return c;
    }
    line[i] = c;
    if (i+1<length) i++;
  }
}

/* ================================================================ */

int matchInt(FILE* inf, int* N)
{
  /* ignore any leading whitespace */
  char sign = ' ';
  for (;;) {
    int d = fgetc(inf);
    if (' ' == d) continue;
    if ('\t' == d) continue;
    if ('-' == d) {
      sign = '-';
      break;
    }
    /* Digit or not; either way put it back */
    ungetc(d, inf);
    if ((d < '0') || (d > '9')) {
      /* Not a digit */
      return 0;
    }
    break;
  }

  *N = 0;

  int numDigits;
  for (numDigits = 0; ; numDigits++) {
    int d = fgetc(inf);
    if ((d < '0') || (d > '9')) {
      /* Not a digit.  Put it back and break out. */
      ungetc(d, inf);
      if ('-' == sign) *N *= -1;
      return numDigits;
    }
    /* Proper digit; increment our number */
    *N *= 10;
    *N += (d - '0');
  }
}

/* ================================================================ 
   |                                                              |
   |                           parse_T                            |
   |                                                              |
   ================================================================ */

int parse_T(FILE* inf)
{
  /* For now - only one type, so just try to match characters */
  if (' ' != matchChar(inf, " ")) return -1;
  if ('s' != matchChar(inf, "s")) return 0;
  if ('i' != matchChar(inf, "i")) return 0;
  if ('m' != matchChar(inf, "m")) return 0;
  if ('p' != matchChar(inf, "p")) return 0;
  if ('l' != matchChar(inf, "l")) return 0;
  if ('e' != matchChar(inf, "e")) return 0;
  ignoreLine(inf);
  return 1;
}

/* ================================================================ 
   |                                                              |
   |                           parse_F                            |
   |                                                              |
   ================================================================ */


int parse_F(FILE* inf, forest_t* F)
{
  /*
    Read forest id
  */
  int fid;
  if (!matchInt(inf, &fid)) {
    ignoreLine(inf);
    return 0;
  }
  if (F) F->fid = fid;
#ifdef DEBUG_PARSER
  printf("    F: got fid %d\n", fid);
#endif
  if (' ' != matchChar(inf, " ")) return 0;

  /* 
    Read optional forest name in quotes 
  */
  const int buflen = 60;
  char buffer[buflen];
  int c = fgetc(inf);
  if ('"' == c) {
    if ('"' != readString(inf, buffer, buflen)) {
      return 0;
    }
    if (F) F->name = strdup(buffer);
#ifdef DEBUG_PARSER
    printf("    F: got name `%s'\n", buffer);
#endif
  } else {
    if (F) F->name = 0;
    ungetc(c, inf);
  }

  /*
    Read left point
  */
  int left;
  if (!matchInt(inf, &left)) {
    ignoreLine(inf);
    return 0;
  }
  if (F) F->left = left;
#ifdef DEBUG_PARSER
  printf("    F: got left %d\n", left);
#endif

  /*
    Read right point
  */
  int right;
  if (!matchInt(inf, &right)) {
    ignoreLine(inf);
    return 0;
  }
  if (F) F->right = right;
#ifdef DEBUG_PARSER
  printf("    F: got right %d\n", right);
#endif

  /*
    Read and allocate initialization array
  */
  if (' ' != matchChar(inf, " ")) return 0;
  if ('[' != matchChar(inf, "[")) return 0;
  if (F) {
    F->counts_raw = (int*)malloc(sizeof(int) * (right-left+1));
    F->counts = F->counts_raw - left;
  }
  for (int i=left; i<=right; i++) {
    int val;
    if (!matchInt(inf, &val)) {
      ignoreLine(inf);
      return 0;
    }
    if (F) F->counts[i] = val;
    if (i<right) if (',' != matchChar(inf, ",")) return 0;
  }
  if (']' != matchChar(inf, "]")) return 0;

  /*
    Ignore rest of line if anyththing
  */
  ignoreLine(inf);
  return 1;
}


/* ================================================================ 
   |                                                              |
   |                           parse_p                            |
   |                                                              |
   ================================================================ */


int parse_p(FILE* inf, int* fid, char* pstr, int plen)
{
  if (!matchInt(inf, fid))        return 0;
  if (' ' != matchChar(inf, " ")) return 0;

  if (0==pstr) plen = 0;
  int i;
  plen--;
  for (i=0; i<plen; i++) {
    int c = fgetc(inf);
    if (EOF == c || '\n' == c || '\r' == c) {
      ungetc(c, inf);
      break;
    }
    pstr[i] = c; 
  }
  if (pstr) pstr[i] = 0;
  ignoreLine(inf);
  return 1;
}



/* ================================================================ 
   |                                                              |
   |                           parse_a                            |
   |                                                              |
   ================================================================ */


update_t* parse_a(FILE* inf)
{
  update_t* list = 0;
  for (;;) {
    int c = fgetc(inf);
    if (' ' == c)   continue;
    if ('\r' == c)  continue;
    if ('\t' == c)  continue;
    if ('\n' == c)  return list;
    if (EOF == c)   return list;

    if ('t' == c) {
      /* Next thing is timestamp. */
      break;
    }

    /* TBD - c should be a digit */
    ungetc(c, inf);

    /* Next should be a triple of integers */
    int fid, level, delta;
    if (!matchInt(inf, &fid)) {
      ignoreLine(inf);
      return list;
    }
    if (!matchInt(inf, &level)) {
      ignoreLine(inf);
      return list;
    }
    if (!matchInt(inf, &delta)) {
      ignoreLine(inf);
      return list;
    }
    list = new_update(fid, level, delta, list);
  }

  /*
    TBD - process timestamp
  */
  ignoreLine(inf);
  return list;
}


