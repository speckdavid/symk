
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


#ifndef HM_NONE_H
#define HM_NONE_H

#include "holeman.h"

namespace MEDDLY {
  class hm_none;
};

/** No hole management.

    Holes are never tracked.  The only thing we try to do to
    reclaim holes is move the "last" pointer forward when
    there are holes in the last slots.

    Holes are represented as follows:
    ---------------------------------------
    [0] -size (number of slots in hole)     
    :
    :
    [size-1] -size

*/
class MEDDLY::hm_none : public holeman {
  public:
    hm_none();
    virtual ~hm_none();

  public:
    virtual node_address requestChunk(int slots);
    virtual void recycleChunk(node_address addr, int slots);
    virtual void dumpInternalInfo(output &s) const;
    virtual void dumpHole(output &s, node_address a) const;
    virtual void reportStats(output &s, const char* pad, unsigned flags) const;
    virtual void clearHolesAndShrink(node_address new_last, bool shrink);

};

#endif

