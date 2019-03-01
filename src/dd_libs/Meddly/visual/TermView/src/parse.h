
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

#ifndef PARSER_H
#define PARSER_H

#include "data.h"

/**
  Ignore the next line of input.
    @param  inf   Input file stream.
*/
void ignoreLine(FILE* inf);


/**
  Parse a 'T' record.
    @param  inf   Input file stream.

    @return   The file type:
               -1 - parse error
                0 - unknown
                1 - simple
*/
int parse_T(FILE* inf);


/** 
  Parse a 'F' record.
    @param  inf   Input file stream.
    @param  F     Where to store the forest info.

    @return 1 on success, 0 on error
*/
int parse_F(FILE* inf, forest_t* F);


/**
  Parse a 'p' record.
    @param  inf     Input file stream.
    @param  fid     Place to store forest id
    @param  pstr    Buffer to store the string
    @param  plen    Length of pstr buffer

    @return 1 on success, 0 on error
*/
int parse_p(FILE* inf, int* fid, char* pstr, int plen);


/** 
  Parse an 'a' record.
    @param  inf     Input file stream.
    
    @return List of updates that were parsed out of the line.
*/
update_t* parse_a(FILE* inf);

#endif
