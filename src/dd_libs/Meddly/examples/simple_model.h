
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

#ifndef SIMPLE_MODEL

/** Build the monolithic next-state function for the given model.
      @param  events    Array of dimension \a nEvents.
                        Each entry is a string where
                        str[j] tells how this event affects variable j:
                        '+' means the event increases the variable by one
                            (unless this would violate the bound)
                        '-' means the event decreases the variable by one
                            (unless the value is already zero)
                        '.' means no change.
      @param  nEvents   Size of events array.
      @param  f         The forest to use; should be a boolean one for relations.
      @param  e         Edge to store the result.
      @param  verb      Verbosity level.
*/
void buildNextStateFunction(const char* const* events, int nEvents, 
  MEDDLY::forest* f, MEDDLY::dd_edge &e, int verb);

/** Build a partitioned next-state function for the given model.
      @param  events    Array of dimension \a nEvents.
                        Each entry is a string where
                        str[j] tells how this event affects variable j:
                        '+' means the event increases the variable by one
                            (unless this would violate the bound)
                        '-' means the event decreases the variable by one
                            (unless the value is already zero)
                        '.' means no change.
      @param  nEvents   Size of events array.
      @param  pnsf      Structure to store the relation.
      @param  verb      Verbosity level.
*/
void buildNextStateFunction(const char* const* events, int nEvents, 
  MEDDLY::satpregen_opname::pregen_relation* pnsf, int verb);

/** Use explicit search to build the reachability set.
      @param  events    Array of dimension \a nEvents.
                        Same format as buildNextStateFunction().
      @param  nEvents   Size of events array.
      @param  f         The forest to use for the reachability set.
      @param  init      The initial state to use; belongs to f.
      @param  e         Edge for storing the result.
      @param  batchsize Number of states to accumulate before each
                        union operation.  If this is less than 1,
                        we use a default size of 256.
*/
void explicitReachset(const char* const* events, int nEvents, 
  MEDDLY::forest* f, MEDDLY::dd_edge &init, MEDDLY::dd_edge &e, int batchsize);

#endif

