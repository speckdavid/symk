
// $Id:$

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
#include "../defines.h"
#include "init_forests.h"

#include "mt.h"
#include "ev.h"

MEDDLY::forest_initializer::forest_initializer(initializer_list *p) : initializer_list(p)
{
}

void MEDDLY::forest_initializer::setup()
{
  forest::mddDefaults.useDefaults(false);
  forest::mxdDefaults.useDefaults(true);

  mt_forest::initStatics();
  ev_forest::initStatics();
}

void MEDDLY::forest_initializer::cleanup()
{
  mt_forest::clearStatics();
  ev_forest::clearStatics();
}

