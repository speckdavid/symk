
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


#include "defines.h"

#include "storage/ct_classic.h"

// **********************************************************************
// *                                                                    *
// *                         ct_object  methods                         *
// *                                                                    *
// **********************************************************************

MEDDLY::ct_object::ct_object()
{
}

MEDDLY::ct_object::~ct_object()
{
}

// ******************************************************************
// *                                                                *
// *                     ct_initializer  methods                    *
// *                                                                *
// ******************************************************************

MEDDLY::ct_initializer::settings MEDDLY::ct_initializer::the_settings;
const MEDDLY::compute_table_style* MEDDLY::ct_initializer::ct_factory;
MEDDLY::compute_table_style* MEDDLY::ct_initializer::builtin_ct_factory;

MEDDLY::ct_initializer::ct_initializer(initializer_list* prev) : initializer_list(prev)
{
  ct_factory = 0;
  builtin_ct_factory = 0;

  setBuiltinStyle(MonolithicUnchainedHash);
  setMaxSize(16777216);
  setStaleRemoval(Moderate);
}

MEDDLY::ct_initializer::~ct_initializer()
{
  delete builtin_ct_factory;
  builtin_ct_factory = 0;
}

void MEDDLY::ct_initializer::setup()
{
  if (0==ct_factory) throw error(error::INVALID_ASSIGNMENT);

  if (ct_factory->usesMonolithic()) {
    operation::Monolithic_CT = ct_factory->create(the_settings);
  }
}

void MEDDLY::ct_initializer::cleanup()
{
  delete operation::Monolithic_CT;
  operation::Monolithic_CT = 0;
}

void MEDDLY::ct_initializer::setStaleRemoval(staleRemovalOption sro)
{
  the_settings.staleRemoval = sro;
}

void MEDDLY::ct_initializer::setMaxSize(unsigned ms)
{
  the_settings.maxSize = ms;
}

void MEDDLY::ct_initializer::setBuiltinStyle(builtinCTstyle cts)
{
  delete builtin_ct_factory;
  builtin_ct_factory = 0;
  switch (cts) {
    case MonolithicUnchainedHash:
          builtin_ct_factory = new monolithic_unchained_style;
          break;

    case MonolithicChainedHash:
          builtin_ct_factory = new monolithic_chained_style;
          break;

    case OperationUnchainedHash:
          builtin_ct_factory = new operation_unchained_style;
          break;

    case OperationChainedHash:
          builtin_ct_factory = new operation_chained_style;
          break;

    case OperationMap:
          builtin_ct_factory = new operation_map_style;
          break;
  }

  ct_factory = builtin_ct_factory;
}

void MEDDLY::ct_initializer::setUserStyle(const compute_table_style* cts)
{
  delete builtin_ct_factory;
  builtin_ct_factory = 0;
  ct_factory = cts;
}

MEDDLY::compute_table* MEDDLY::ct_initializer::createForOp(operation* op)
{
  if (ct_factory) {
    return ct_factory->create(the_settings, op);
  } else {
    return 0;
  }
}

// **********************************************************************
// *                                                                    *
// *                    compute_table_style  methods                    *
// *                                                                    *
// **********************************************************************

MEDDLY::compute_table_style::compute_table_style()
{
}

MEDDLY::compute_table_style::~compute_table_style()
{
}

MEDDLY::compute_table* 
MEDDLY::compute_table_style::create(const ct_initializer::settings &s)
      const
{
  throw error(error::TYPE_MISMATCH);
}


MEDDLY::compute_table* 
MEDDLY::compute_table_style::create(const ct_initializer::settings &s, 
      operation* op) const
{
  throw error(error::TYPE_MISMATCH);
}

// **********************************************************************
// *                                                                    *
// *                       compute_table  methods                       *
// *                                                                    *
// **********************************************************************

MEDDLY::compute_table::compute_table(const ct_initializer::settings &s)
{
  maxSize = s.maxSize;
  if (0==maxSize)
    throw error(error::INVALID_ASSIGNMENT);

  switch (s.staleRemoval) {
    case ct_initializer::Aggressive:
            checkStalesOnFind = true;
            checkStalesOnResize = true;
            break;
    case ct_initializer::Moderate:
            checkStalesOnFind = false;
            checkStalesOnResize = true;
            break;
    case ct_initializer::Lazy:
            checkStalesOnFind = false;
            checkStalesOnResize = false;
            break;
  }

  perf.numEntries = 0;
  perf.hits = 0;
  perf.pings = 0;
  perf.numLargeSearches = 0;
  perf.maxSearchLength = 0;
  for (int i=0; i<perf.searchHistogramSize; i++)
    perf.searchHistogram[i] = 0;
}

MEDDLY::compute_table::~compute_table()
{
}

// **********************************************************************

MEDDLY::compute_table::search_key::search_key(operation* _op)
{
  op = _op;
}

MEDDLY::compute_table::search_key::~search_key()
{
}

// **********************************************************************

MEDDLY::compute_table::search_result::search_result()
{

}
MEDDLY::compute_table::search_result::~search_result()
{
}

// **********************************************************************

MEDDLY::compute_table::entry_builder::entry_builder()
{

}
MEDDLY::compute_table::entry_builder::~entry_builder()
{
}

