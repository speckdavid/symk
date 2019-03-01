
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

#ifndef PREPOSTIMAGE_H
#define PREPOSTIMAGE_H

namespace MEDDLY {
  class binary_opname;

  /// Set up a binary_opname for the "preimage" operation.
  binary_opname* initializePreImage();

  /// Set up a binary_opname for the "postimage" operation.
  binary_opname* initializePostImage();

  /// Set up a binary_opname for vector matrix multiplication.
  binary_opname* initializeVMmult();

  /// Set up a binary_opname for matrix vector multiplication.
  binary_opname* initializeMVmult();
}

#endif

