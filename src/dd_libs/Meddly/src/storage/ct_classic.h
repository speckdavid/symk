
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

#ifndef CT_CLASSIC_H
#define CT_CLASSIC_H

#include "../defines.h"


// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                        Compute Table styles                        *
// *                                                                    *
// *                                                                    *
// **********************************************************************

namespace MEDDLY {
  class monolithic_chained_style;
  class monolithic_unchained_style;
  class operation_chained_style;
  class operation_unchained_style;
  class operation_map_style;
};

// **********************************************************************
// *                                                                    *
// *                   monolithic_chained_style class                   *
// *                                                                    *
// **********************************************************************

class MEDDLY::monolithic_chained_style : public compute_table_style {
  public:
    monolithic_chained_style();
    virtual compute_table* create(const ct_initializer::settings &s) const;
    virtual bool usesMonolithic() const;
};

// **********************************************************************
// *                                                                    *
// *                  monolithic_unchained_style class                  *
// *                                                                    *
// **********************************************************************

class MEDDLY::monolithic_unchained_style : public compute_table_style {
  public:
    monolithic_unchained_style();
    virtual compute_table* create(const ct_initializer::settings &s) const;
    virtual bool usesMonolithic() const;
};

// **********************************************************************
// *                                                                    *
// *                   operation_chained_style  class                   *
// *                                                                    *
// **********************************************************************

class MEDDLY::operation_chained_style : public compute_table_style {
  public:
    operation_chained_style();
    virtual compute_table* create(const ct_initializer::settings &s, operation* op) const;
    virtual bool usesMonolithic() const;
};

// **********************************************************************
// *                                                                    *
// *                  operation_unchained_style  class                  *
// *                                                                    *
// **********************************************************************

class MEDDLY::operation_unchained_style : public compute_table_style {
  public:
    operation_unchained_style();
    virtual compute_table* create(const ct_initializer::settings &s, operation* op) const;
    virtual bool usesMonolithic() const;
};

// **********************************************************************
// *                                                                    *
// *                     operation_map_style  class                     *
// *                                                                    *
// **********************************************************************

class MEDDLY::operation_map_style : public compute_table_style {
  public:
    operation_map_style();
    virtual compute_table* create(const ct_initializer::settings &s, operation* op) const;
    virtual bool usesMonolithic() const;
};

#endif
