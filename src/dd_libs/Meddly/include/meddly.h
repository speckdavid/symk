  
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


/*! \file meddly.h 

    MDD library interface.
    
    This interface is enough for "casual" users, i.e., users
    who are happy to use only the built-in operations.
    There is also an "expert" interface for users who would
    like to define new operations (in fact, the built-in
    operations use this same interface).
    
    General naming principles:
    Class names are of the form class_name.
    Method names are of the form verbObjectDetails.

*/

#ifndef MEDDLY_H
#define MEDDLY_H

#ifndef _MEDDLY_WITHOUT_CSTDIO_
#include <cstdio>
#endif

#ifndef _MEDDLY_WITHOUT_IOSTREAM_
#include <iostream>
#endif

#include <vector>
#include <memory>
#include <cassert>

namespace MEDDLY {
  /** Special value for minterms: don't care what this variable does.
      I.e., do the same thing for all possible assignments for a variable.
  */
  const int DONT_CARE  = -1;
  /** Special value for primed minterms: don't change this variable.
      Forces the primed value to equal the unprimed value for a variable.
      Undefined for unprimed variables.
  */
  const int DONT_CHANGE = -2;


  // Typedefs
  typedef unsigned char node_storage_flags;

  /** Handles for nodes.
      This should be either int or long, and effectively limits
      the number of possible nodes per forest.
      As an int, we get 2^32-1 possible nodes per forest,
      which should be enough for most applications.
      As a long on a 64-bit machine, we get 2^64-1 possible nodes
      per forest, at the expense of nearly doubling the memory used.
      This also specifies the incoming count range for each node.
  */
  typedef int  node_handle;

  /** Node addresses.
      This is used for internal storage of a node,
      and should probably not be changed.
      The typedef is given simply to clarify the code
      (hopefully :^)
  */
  typedef long node_address;

  // Classes

  class error;
  class input;
  class FILE_input;
  class istream_input;
  class output;
  class FILE_output;
  class ostream_output;

  class forest;
  class expert_forest;
  class unpacked_node;

  class node_storage_style;

  class variable;
  class variable_order;
  class domain;
  class dd_edge;
  class enumerator;
  class ct_object;
  class unary_opname;
  class binary_opname;
  class operation;
  class unary_operation;
  class binary_operation;

  /// Argument and result types for apply operations.
  enum opnd_type {
    FOREST      = 0,
    BOOLEAN     = 1,
    INTEGER     = 2,
    REAL        = 3,
    HUGEINT     = 4,
    FLOATVECT   = 5,
    DOUBLEVECT  = 6
  };

  // ******************************************************************
  // *                    miscellaneous  functions                    *
  // ******************************************************************

#ifdef __GMP_H__
  ct_object& get_mpz_wrapper();
  void unwrap(const ct_object &, mpz_t &value);
#endif

  // ******************************************************************
  // *                     Node storage mechanisms                    *
  // ******************************************************************

  /** "Classic" node storage mechanism.
      The node storage mechanism from early versions of this library.
  */
  extern const node_storage_style* CLASSIC_STORAGE;

  /** Similar to classic.
      Differences are in class design, so we can measure overhead
      (if any) of class design differences.
  */
  extern const node_storage_style* SIMPLE_GRID;

  /** Like classic, but use an array of lists for hole management.
  */
  extern const node_storage_style* SIMPLE_ARRAY;

  /** Like classic, but use heaps for hole management.
  */
  extern const node_storage_style* SIMPLE_HEAP;

  /** Classic node storage but no hole management.
  */
  extern const node_storage_style* SIMPLE_NONE;

  /** Nodes are stored in a compressed form.
      Holes are managed using the original grid structure.
  */
  extern const node_storage_style* COMPACT_GRID;


  // ******************************************************************
  // *                     Named unary operations                     *
  // ******************************************************************

  /** Create a copy of a dd_edge.
      The copy may be stored in any forest as long as it belongs to the
      same domain as the original and the transformation is valid.
      Copying is valid with the following:
      MDD to MTMDD, MTMDD to MDD, MXD to MTMXD, MTMXD to MXD.
  */
  extern const unary_opname* COPY;

  /// Unary operation.  Return the number of variable assignments 
  /// so that the function evaluates to non-zero.
  extern const unary_opname* CARDINALITY;

  /// For BOOLEAN forests, flip the return values.
  extern const unary_opname* COMPLEMENT;

  /// Find the largest value returned by the function.
  extern const unary_opname* MAX_RANGE;

  /// Find the smallest value returned by the function.
  extern const unary_opname* MIN_RANGE;

  /// Convert MDD to EV+MDD index set.  A special case of COPY, really.
  extern const unary_opname* CONVERT_TO_INDEX_SET;

  // ******************************************************************
  // *                    Named  binary operations                    *
  // ******************************************************************

  /// Set operation for forests with range_type of BOOLEAN. All operands
  /// must belong to the same forest.
  extern const binary_opname* UNION;
  /// Set operation for forests with range_type of BOOLEAN. All operands
  /// must belong to the same forest.
  extern const binary_opname* INTERSECTION;
  /// Set operation for forests with range_type of BOOLEAN. All operands
  /// must belong to the same forest.
  extern const binary_opname* DIFFERENCE;

  /// Binary operation.  Combines two functions into a single one,
  /// where the operands are MDDs and the result is an MXD.
  /// Specifically, for MDD operands f and g, produces MXD h where
  /// h(xn, x'n, ..., x1, x'1) = f(xn, ..., x1) * g(x'n, ..., x'1)
  /// Works for BOOLEAN forests.
  extern const binary_opname* CROSS;

  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* MINIMUM;
  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* MAXIMUM;
  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* PLUS;
  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* MINUS;
  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* MULTIPLY;
  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* DIVIDE;
  /// For forests with range_type of INTEGER. All operands must
  /// belong to the same forest.
  extern const binary_opname* MODULO;

  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* EQUAL;
  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* NOT_EQUAL;
  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* LESS_THAN;
  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* LESS_THAN_EQUAL;
  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* GREATER_THAN;
  /// For forests with range_type of INTEGER and REAL. All operands must
  /// belong to the same forest.
  extern const binary_opname* GREATER_THAN_EQUAL;

  /** Image operations on a set-of-states with a transition function.
      The first operand must be the set-of-states and the second operand
      must be the transition function. The result is a set-of-states that
      must be stored in the same forest as the first operand.
      
      Applies to:
      PRE_IMAGE, POST_IMAGE, REACHABLE_STATES_DFS, REACHABLE_STATES_BFS,
      REVERSE_REACHABLE_DFS, REVERSE_REACHABLE_BFS.
  */
  extern const binary_opname* PRE_IMAGE;
  extern const binary_opname* POST_IMAGE;
  extern const binary_opname* REACHABLE_STATES_DFS;
  extern const binary_opname* REACHABLE_STATES_BFS;
  extern const binary_opname* REVERSE_REACHABLE_DFS;
  extern const binary_opname* REVERSE_REACHABLE_BFS;

  /** Vector matrix multiply, where the first argument is vector (MDD),
      the second argument is a matrix (MXD), and the result is a vector (MDD).
  */
  extern const binary_opname* VM_MULTIPLY;

  /** Matrix vector multiply, where the first argument is a matrix (MXD),
      the second argument is a vector (MDD), and the result is a vector (MDD).
  */
  extern const binary_opname* MV_MULTIPLY;

  /** Matrix multiplication, where the first argument is a matrix (MXD),
      the second argument is a matrix (MXD), and the result is a matrix (MXD),
      such that, C[m][n] += A[m][i] * B[i][n], for all m, n and i.
  */
  extern const binary_opname* MM_MULTIPLY;

  // ******************************************************************
  // *                  library management functions                  *
  // ******************************************************************

  /** Initialize the library with default settings.
      See meddly_expert.h for functions to initialize 
      the library with non-default settings.
      Should be called before using any other functions.
  */
  void initialize();

  /** Clean up the library.
      Can be called to free memory used by the library;
      after it is called, the library may be initialized again.
  */
  void cleanup();

  /** Get the information about the library.
      @param  what  Determines the type of information to obtain.
      @return A human-readable information string.
              The string depends on parameter \a what, as follows.
              0: Title string, e.g., "MDD library version 0.0.0"
              1: Copyright string
              2: License string
              3: Reference url
              4: String with library features
              5: Release date
              Anything else: null string.
  */
  const char* getLibraryInfo(int what = 0);

  // ******************************************************************
  // *                   object creation  functions                   *
  // ******************************************************************

  /** Front-end function to create a variable.
      This is required because variable is an abstract class.
        @param  bound   The initial bound for the varaible.
        @param  name    Variable name (used only in display / debugging), or 0.
        @return A new variable, or 0 on error.
  */
  variable* createVariable(int bound, char* name);

  /** Front-end function to create a domain with the given variables.
        @param  vars    List of variables, in order.
                        vars[i] gives the variable at level i.
                        Note that vars[0] should be 0.
        @param  N       Number of variables.
                        vars[N] refers to the top-most variable.

        @return A new domain.
  */
  domain* createDomain(variable** vars, int N);

  /** Front-end function to create an empty domain.
      This is required because domain is an abstract class.
  */
  domain* createDomain();

  /** Front-end function to create a domain with given variable bounds.
      Equivalent to creating an empty domain and then building the
      domain bottom up.
  
        @param  bounds  variable bounds.
                        bounds[i] gives the bound for the variable
                        at level i+1.
        @param  N       Number of variables.

        @return A new domain.
  */
  domain* createDomainBottomUp(const int* bounds, int N);

/* Commented out as of version 0.10
#ifdef _MSC_VER
  __declspec(deprecated)
#endif
#ifdef __GNUC__
  __attribute__ ((deprecated))
#endif
  /// This function is deprecated as of version 0.4; 
  /// use "createDomain" instead.
  domain* MEDDLY_createDomain();
*/


  // ******************************************************************
  // *                  object destruction functions                  *
  // ******************************************************************

  /** Front-end function to destroy a domain.
      For consistency.
  */
  void destroyDomain(domain* &d);

  /** Front-end function to destroy a forest.
  */
  void destroyForest(forest* &f);

  // ******************************************************************
  // *                          Unary  apply                          *
  // ******************************************************************

  /** Apply a unary operator.
      The operand and the result are not necessarily in the same forest,
      but they must belong to forests that share the same domain.
      This is useful, for instance, for copying a function to a new forest.
        @param  op    Operator handle.
        @param  a     Operand.
        @param  c     Output parameter: the result, where \a c = \a op \a a.
  */
  void apply(const unary_opname* op, const dd_edge &a, dd_edge &c);

  /** Apply a unary operator.
      For operators whose result is an integer.
        @param  op    Operator handle.
        @param  a     Operand.
        @param  c     Output parameter: the result, where \a c = \a op \a a.
  */
  void apply(const unary_opname* op, const dd_edge &a, long &c);

  /** Apply a unary operator.
      For operators whose result is a real.
        @param  op    Operator handle.
        @param  a     Operand.
        @param  c     Output parameter: the result, where \a c = \a op \a a.
  */
  void apply(const unary_opname* op, const dd_edge &a, double &c);

  void apply(const unary_opname* op, const dd_edge &a, opnd_type cr, 
    ct_object &c);

#ifdef __GMP_H__
  /** Apply a unary operator.
      For operators whose result is an arbitrary-precision integer
      (as supplied by the GNU MP library).
        @param  op    Operator handle.
        @param  a     Operand.
        @param  c     Input: an initialized MP integer.
                      Output parameter: the result, where \a c = \a op \a a.
  */
  void apply(const unary_opname* op, const dd_edge &a, mpz_t &c);
#endif

  // ******************************************************************
  // *                          Binary apply                          *
  // ******************************************************************

  /** Apply a binary operator.
      \a a, \a b and \a c are not required to be in the same forest,
      but they must have the same domain. The result will be in the
      same forest as \a result. The operator decides the type of forest
      for each \a dd_edge.
      Useful, for example, for constructing comparisons
      where the resulting type is "boolean" but the operators are not,
      e.g., c = f EQUALS g.
      @param  op    Operator handle.
      @param  a     First operand.
      @param  b     Second operand.
      @param  c     Output parameter: the result,
                    where \a c = \a a \a op \a b.
  */ 
  void apply(const binary_opname* op, const dd_edge &a, const dd_edge &b,
    dd_edge &c);


}  // namespace MEDDLY


// ******************************************************************
// ******************************************************************
// ******************************************************************
// ******************************************************************
// ******************************************************************


// ******************************************************************
// *                          error  class                          *
// ******************************************************************
 
/// Class for errors thrown by MEDDLY.
class MEDDLY::error {
  public:
    /// Error codes.
    enum code {
      /// The library was not initialized.
      UNINITIALIZED,
      /// The library was already initialized.
      ALREADY_INITIALIZED,
      /// The requested operation is not yet implemented!
      NOT_IMPLEMENTED,
      /// An operation failed due to lack of memory.
      INSUFFICIENT_MEMORY,
      /// An operation is not supported for the given forest.
      INVALID_OPERATION,
      /// A provided variable is erroneous.
      INVALID_VARIABLE,
      /// A provided level number is erroneous.
      INVALID_LEVEL,
      /// A provided variable bound is out of range.
      INVALID_BOUND,
      /// We expected an empty domain, but it wasn't
      DOMAIN_NOT_EMPTY,
      /// Unknown operation (bad operation handle).
      UNKNOWN_OPERATION,
      /// Requested operation requires same domains, they weren't.
      DOMAIN_MISMATCH,
      /// Requested operation requires same forest, it wasn't.
      FOREST_MISMATCH,
      /// Requested operation not supported for operand or result type.
      TYPE_MISMATCH,
      /// Requested operation requires different number of operands.
      WRONG_NUMBER,
      /// A result won't fit in an integer / float.
      VALUE_OVERFLOW,
      /// Integer division by 0 is invalid.
      DIVIDE_BY_ZERO,
      /// Invalid policy setting.
      INVALID_POLICY,
      /// Bad value for something.
      INVALID_ASSIGNMENT,
      /// Invalid argument (for specialized operations)
      INVALID_ARGUMENT,
      /// File format error.
      INVALID_FILE,
      /// File input error.
      COULDNT_READ,
      /// File output error.
      COULDNT_WRITE,
      /// Miscellaneous error
      MISCELLANEOUS
    };
  public:
    error(code c);
    code getCode() const;
    const char* getName() const;
  private:
    code errcode;
};


// ******************************************************************
// ******************************************************************
// *                                                                *
// *                                                                *
// *                       MEDDLY  custom I/O                       *
// *                                                                *
// ******************************************************************
// 
// We are using our own I/O classes because we want to support both C++ streams
// and C FILE*s, and this seems to be the cleanest way to do that.  Also it
// allows users to (easily?) define their own I/O mechanisms.                               
//
// Implementation is in io.cc
// 
// ******************************************************************
// ******************************************************************

// ******************************************************************
// *                                                                *
// *                          input  class                          *
// *                                                                *
// ******************************************************************

/** Input class.
    Abstract base class.
*/
class MEDDLY::input {
  public:
    input();
    virtual ~input();

    /**
        Return true if and only if the input stream has hit EOF.
    */
    virtual bool eof() const = 0;

    /**
        Read exactly one character from the input stream.
          @return   The character consumed, or EOF.
          @throws   an appropriate error
    */
    virtual int get_char() = 0;

    /**
        Put the last consumed character back on the stream.
        Does nothing if no character was consumed.
          @param  x   Character to put back.
                      x will be the last consumed character.
          @throws     an appropriate error
    */
    virtual void unget(char x) = 0;

    /**
        Read an integer in signed decimal notation.
        TBD - integer is terminated by whitespace, or any character?
          @return   The integer consumed
          @throws   an appropriate error
    */
    virtual long get_integer() = 0;

    /**
        Read a floating point value in signed decimal notation.
        TBD - value is terminated by whitespace, or any character?
          @return   The value consumed
          @throws   an appropriate error
    */
    virtual double get_real() = 0;

    /**
        Read raw bytes into a memory location.
          @param  bytes   Number of bytes requested
          @param  buffer  Pointer to store the bytes

          @return Number of bytes actually read, 
                  or negative on error.
    */
    virtual int read(int bytes, unsigned char* buffer) = 0;


  /*
    Handy input stream manipulation
  */

  /**
      Consume whitespace (if any) from the input stream,
      including comments of the form #...\n
  */
  void stripWS();

  /**
      Consume a keyword from the input stream.
      If the keyword does not match the input stream,
      we throw an INVALID_FILE error.
  */
  void consumeKeyword(const char* keyword);
};  // end of input class

// ******************************************************************
// *                                                                *
// *                        FILE_input class                        *
// *                                                                *
// ******************************************************************

#ifndef _MEDDLY_WITHOUT_CSTDIO_

/** FILE_Input class.
    Use for C-style FILE* input.

    TBD - come up with a way to remove #include <cstdio> and <iostream> from above!
*/
class MEDDLY::FILE_input : public MEDDLY::input {
  public:
    FILE_input(FILE* _inf);
    virtual ~FILE_input();

    virtual bool eof() const;
    virtual int get_char();
    virtual void unget(char);
    virtual long get_integer();
    virtual double get_real();
    virtual int read(int bytes, unsigned char* buffer);

  private:
    FILE* inf;
};  // end of FILE_input class

#endif

// ******************************************************************
// *                                                                *
// *                      istream_input  class                      *
// *                                                                *
// ******************************************************************

#ifndef _MEDDLY_WITHOUT_IOSTREAM_

/** istream_Input class.
    Use for C++-style istream input.

    TBD - come up with a way to remove #include <cstdio> and <iostream> from above!
*/
class MEDDLY::istream_input : public MEDDLY::input {
  public:
    istream_input(std::istream &in);
    virtual ~istream_input();

    virtual bool eof() const;
    virtual int get_char();
    virtual void unget(char);
    virtual long get_integer();
    virtual double get_real();
    virtual int read(int bytes, unsigned char* buffer);

  private:
    std::istream &in;
};  // end of istream_input class

#endif

// ******************************************************************
// *                                                                *
// *                          output class                          *
// *                                                                *
// ******************************************************************

/** Output class.
    Abstract base class.
*/
class MEDDLY::output {
  public:
    output();
    virtual ~output();

    /**
        Write exactly one character to the output stream.
          @param  x   Character to write
          @throws     An appropriate error
    */
    virtual void put(char x) = 0;

    /**
        Write a null-terminated string to the output stream.
        The terminating null is not written.
          @param  x   String to write
          @param  w   Width for formatting
          @throws     An appropriate error
    */
    virtual void put(const char* x, int w=0) = 0;

    /**
        Write a signed, decimal integer to the output stream.
          @param  x   Integer to write
          @param  w   Width for formatting
          @throws     An appropriate error
    */
    virtual void put(long x, int w=0) = 0;

    /**
        Write hex digits to the output stream.
          @param  x   Value to write
          @param  w   Width for formatting
          @throws     An appropriate error
    */
    virtual void put_hex(unsigned long x, int w=0) = 0;

    /**
        Write a signed, decimal, floating-point value to the output stream.
          @param  x   Value to write
          @param  w   Width
          @param  p   Precision
          @param  f   Format, either 'e', 'f', or 'g' (for the style of printf)
          @throws     An appropriate error
    */
    virtual void put(double x, int w=0, int p=6, char f='g') = 0;

    /**
        Write raw bytes from a memory location.
          @param  bytes   Number of bytes in the buffer
          @param  buffer  Pointer to memory location
          
          @return Number of bytes actually written,
                  or negative on error.
    */
    virtual int write(int bytes, const unsigned char* buffer) = 0;

    /**
        Flush the output stream.
    */
    virtual void flush() = 0;

  /*
    Handy output stream manipulation
  */

    /**
        Write number of bytes, with units.
          @param  m       Number of bytes
          @param  human   If false, units will be "bytes".
                          If true, scale units so that output is betwen one and 1000.
    */
    void put_mem(long m, bool human);

};  // end of output class

/* 
  These will let us do C++ style output, with our class.
*/

inline  MEDDLY::output& operator<< (MEDDLY::output &s, char x)        { s.put(x); return s; }
inline  MEDDLY::output& operator<< (MEDDLY::output &s, const char* x) { s.put(x); return s; }
inline  MEDDLY::output& operator<< (MEDDLY::output &s, int x)         { s.put(long(x)); return s; }
inline  MEDDLY::output& operator<< (MEDDLY::output &s, long x)        { s.put(x); return s; }
inline  MEDDLY::output& operator<< (MEDDLY::output &s, double x)      { s.put(x); return s; }


// ******************************************************************
// *                                                                *
// *                       FILE_output  class                       *
// *                                                                *
// ******************************************************************

#ifndef _MEDDLY_WITHOUT_CSTDIO_

/** FILE_output class.
    Use for C-style FILE* input.

    TBD - come up with a way to remove #include <cstdio> and <iostream> from above!
*/
class MEDDLY::FILE_output : public MEDDLY::output {
  public:
    FILE_output(FILE* outf);
    virtual ~FILE_output();

    virtual void put(char x);
    virtual void put(const char*, int w);
    virtual void put(long x, int w);
    virtual void put_hex(unsigned long x, int w);
    virtual void put(double x, int w, int p, char f);
    virtual int write(int bytes, const unsigned char* buffer);
    virtual void flush();

  private:
    FILE* outf;
};  // end of FILE_output class

#endif

// ******************************************************************
// *                                                                *
// *                      ostream_output class                      *
// *                                                                *
// ******************************************************************

#ifndef MEDDLY_WITHOUT_IOSTREAM_

/** ostream_output class.
    Use for C++-style ostream input.

    TBD - come up with a way to remove #include <cstdio> and <iostream> from above!
*/
class MEDDLY::ostream_output : public MEDDLY::output {
  public:
    ostream_output(std::ostream &out);
    virtual ~ostream_output();

    virtual void put(char x);
    virtual void put(const char*, int w);
    virtual void put(long x, int w);
    virtual void put_hex(unsigned long x, int w);
    virtual void put(double x, int w, int p, char f);
    virtual int write(int bytes, const unsigned char* buffer);
    virtual void flush();

  private:
    std::ostream &out;
};  // end of ostream_output class

#endif

// ******************************************************************
// ******************************************************************
// ******************************************************************
// ******************************************************************

// ******************************************************************
// *                                                                *
// *                                                                *
// *                          forest class                          *
// *                                                                *
// *                                                                *
// ******************************************************************

/** Forest class.
    Abstract base class.
    
    A data structure for managing collections of functions
    (or sets, or vectors, or relations, or matrices,
    depending on your conceptual view) represented in a single
    decision diagram forest over a common domain.
    
    TBD: discussion of garbage collection.

    When a forest is destroyed, all of the corresponding dd_edges
    are also destroyed, as are any compute table entries for the forest.
*/
class MEDDLY::forest {
  public:
    int *level_reduction_rule;
    
    /** Types of values that we can currently store in forests.
        I.e., if every node in a forest is a function,
        these are the possible ranges for a function.
    */
    enum range_type {
      /// boolean-valued functions.
      BOOLEAN,
      /// integer-valued functions.
      INTEGER,
      /// real-valued functions.
      REAL
    };

    /// Edge annotation mechanism.
    enum edge_labeling {
      /// Edges unlabeled, all values stored in distinct terminals.
      MULTI_TERMINAL,
      /// Edges labeled, values summed along path.
      EVPLUS,
      /// Special case of EVPLUS for indexed sets.
      INDEX_SET,
      /// Edges labeled, values multiplied along path.
      EVTIMES
      // TBD: there may be others in the future :^)
    };

    /// Collection of forest policies.
    struct policies {

      /** Supported node reduction rules.
          Currently, the following reduction rules are allowed:
            - Fully reduced, meaning that duplicate and redundant nodes are
              eliminated.
            - Quasi reduced, meaning that duplicate nodes are eliminated.
            - Identity reduced, for relations only, meaning that duplicate
              nodes are eliminated, as are "identity" pairs of primed, unprimed
              variables.
      */
      enum reduction_rule {
          /// Nodes are fully reduced.
          FULLY_REDUCED,
          /// Nodes are quasi-reduced.
          QUASI_REDUCED,
          /// Nodes are identity-reduced.
          IDENTITY_REDUCED,
	 /// Nodes are user-defined reduced
	  USER_DEFINED
      };

      // Supported node storage meachanisms.
      static const unsigned char ALLOW_FULL_STORAGE;
      static const unsigned char ALLOW_SPARSE_STORAGE;

      /// Supported node deletion policies.
      enum node_deletion {
          /** Never delete nodes.
              Useful for debugging the garbage collector.
          */
          NEVER_DELETE,
          /** Nodes are marked for deletion.
              We assume that a deleted node might be used again,
              so we keep it until it no longer appears in any compute table.
          */
          OPTIMISTIC_DELETION,
          /// Nodes are removed as soon as they become disconnected.
          PESSIMISTIC_DELETION
      };

      // Supported variable swap strategies.
      // Work for relations only.
      enum class variable_swap_type {
    	  // Swap adjacent variables in one go.
    	  VAR,
    	  // Swap adjacent variables through swapping levels 4 time.
    	  // Do not work for fully-identity reduced relations.
    	  LEVEL
      };

      enum class reordering_type {
        // Always choose the lowest swappable inversion
        LOWEST_INVERSION,
        // Always choose the highest swappable inversion
        HIGHEST_INVERSION,
        // Sink down the "heaviest" variables
        SINK_DOWN,
        // Bubble up the "lightest" variables
        BRING_UP,
        // Always choose the swappabel inversion where the variable on the top
        // has the fewest associated nodes
        LOWEST_COST,
        // Always choose the swappable inversion that will result in
        // the lowest memory consumption
        LOWEST_MEMORY,
        // Choose the swappable inversion randomly
        RANDOM,
        // Always choose the swappable inversion with the lowest average reference count
        LARC
      };

      /// Defaults: how may we store nodes for all levels in the forest.
      node_storage_flags storage_flags;
      /// Default reduction rule for all levels in the forest.
      reduction_rule reduction;
      /// Default deletion policy for all levels in the forest.
      node_deletion deletion;
      // Default variable reorder strategy.
      reordering_type reorder;
      // Default variable swap strategy.
      variable_swap_type swap;

      /// Backend storage mechanism for nodes.
      const node_storage_style* nodestor;

      /// Memory compactor: never run if fewer than this many unused slots.
      int compact_min;
      /// Memory compactor: always run if more than this many unused slots.
      int compact_max;
      /// Memory compactor: fraction of unused slots to trigger.
      int compact_frac;

      /// Number of zombie nodes to trigger garbage collection
      int zombieTrigger;
      /// Number of orphan nodes to trigger garbage collection
      int orphanTrigger;
      /// Should we run the memory compactor after garbage collection
      bool compactAfterGC;
      /// Should we run the memory compactor before trying to expand
      bool compactBeforeExpand;

      /// Empty constructor, for setting up defaults later
      policies();

      /// Constructor; sets reasonable defaults
      policies(bool rel);

      /// Set to hard-wired defaults
      void useDefaults(bool rel);

      void setFullStorage();
      void setSparseStorage();
      void setFullOrSparse();
      void setFullyReduced();
      void setQuasiReduced();
      void setIdentityReduced();
      void setUserDefinedReduced();
      void setNeverDelete();
      void setOptimistic();
      void setPessimistic();

      void setLowestInversion();
      void setHighestInversion();
      void setSinkDown();
      void setBringUp();
      void setLowestCost();
      void setLowestMemory();
      void setRandom();
      void setLARC();

      void setVarSwap();
      void setLevelSwap();
    }; // end of struct policies

    /// Collection of various stats for performance measurement
    struct statset {
      /// Number of times a dead node was resurrected.
      long reclaimed_nodes;
      /// Number of times the forest storage array was compacted
      long num_compactions;
      /// Number of times the garbage collector ran.
      long garbage_collections;
      /// Current number of zombie nodes (waiting for deletion)
      long zombie_nodes;
      /// Current number of orphan nodes (disconnected)
      long orphan_nodes;
      /// Current number of connected nodes
      long active_nodes;
      /// Current number of temporary nodes
      long temp_nodes;

      /// Peak number of active nodes
      long peak_active;

      /// Current memory used for nodes
      long memory_used;
      /// Current memory allocated for nodes
      long memory_alloc;
      /// Peak memory used for nodes
      long peak_memory_used;
      /// Peak memory allocated for nodes
      long peak_memory_alloc;

      // unique table stats

      /// Current memory for UT
      long memory_UT;
      /// Peak memory for UT
      long peak_memory_UT;
      /// Longest chain search in UT
      int max_UT_chain;

      statset();

      // nice helpers

      void incActive(long b);
      void decActive(long b);
      void incMemUsed(long b);
      void decMemUsed(long b);
      void incMemAlloc(long b);
      void decMemAlloc(long b);
      void sawUTchain(int c);
    };

    /**
        Abstract base class for logging of forest stats.
        The idea is to allow an external tool to read this information,
        and display stats in real time.  Potentially much more detailed
        that the stats collected above.
    */
    class logger {
        bool nfix;
        bool node_counts;
        bool time_stamps;

        long startsec;
        long startusec;
      public:
        logger();
        virtual ~logger();

      /*
          Settings.
          Cannot be changed once we attach to a forest.
      */
      public:
        bool recordingNodeCounts() const;
        void recordNodeCounts();
        void ignoreNodeCounts();

        bool recordingTimeStamps() const;
        void recordTimeStamps();
        void ignoreTimeStamps();

      /*
          Hooks, used in various places.
          Must be overridden in derived classes.
      */
      public:
        /**
            Insert a comment string.
            May be ignored depending on the file format.

              @param  comment   String to insert.
        */
        virtual void addComment(const char* comment) = 0;

        /**
            Start a new phase of computation.
            May be ignored depending on the file format.

              @param  f         Forest this applies to.
              @param  comment   Info to display about this phase.
        */
        virtual void newPhase(const forest* f, const char* comment) = 0;

        /**
            Called once, when the logger is attached to a forest.
            Must call method fixLogger().

              @param  f       Forest info to log
              @param  name    Forest name; better for displaying if we
                              have multiple forests.
        */
        virtual void logForestInfo(const forest* f, const char* name) = 0;

        /**
            Change node counts by specified amounts.
        */
        virtual void addToActiveNodeCount(const forest* f, int level, long delta) = 0;

      /*
          Helpers for derived classes
      */
      protected:
        /* Use this for generating timestamps. */
        void currentTime(long &sec, long &usec);

        /* Call this inside logForestInfo() */
        void fixLogger();
    };


  protected:
    /** Constructor -- this class cannot be instantiated.
      @param  dslot   slot used to store the forest, in the domain
      @param  d       domain to which this forest belongs to.
      @param  rel     does this forest represent a relation.
      @param  t       the range of the functions represented in this forest.
      @param  ev      edge annotation.
      @param  p       Polcies for reduction, storage, deletion.
    */
    forest(int dslot, domain* d, bool rel, range_type t, edge_labeling ev, 
      const policies &p,int* level_reduction_rule);

    /// Destructor.
    virtual ~forest();  

  // ------------------------------------------------------------
  // static inlines.
  public:
    /** Go "down a level" in a relation.
        Safest to use this, in case in later versions
        the level numbering changes, or becomes forest dependent.
            @param  k   Current level
            @return Level immediately below level k
    */
    static int downLevel(int k);

    /** Go "up a level" in a relation.
        Safest to use this, in case in later versions
        the level numbering changes, or becomes forest dependent.
            @param  k   Current level
            @return Level immediately above level k
    */
    static int upLevel(int k);

    /**
        Get the default policies for MDD forests.
    */
    static const policies& getDefaultPoliciesMDDs();

    /**
        Get the default policies for MxD forests.
    */
    static const policies& getDefaultPoliciesMXDs();

    /**
        Set the default policies for MDD forests.
    */
    static void setDefaultPoliciesMDDs(const policies&);

    /**
        Set the default policies for MxD forests.
    */
    static void setDefaultPoliciesMXDs(const policies&);

        

  // ------------------------------------------------------------
  // inlines.
  public:

    /// Returns the forest identifier, a unique integer per forest.
    unsigned FID() const;

    /// Returns a non-modifiable pointer to this forest's domain.
    const domain* getDomain() const;

    /// Returns a pointer to this forest's domain.
    domain* useDomain();

    /// Does this forest represent relations or matrices?
    bool isForRelations() const;

    /// Returns the range type.
    range_type getRangeType() const;

    /// Query the range type.
    bool isRangeType(range_type r) const;

    /// Returns the edge labeling mechanism.
    edge_labeling getEdgeLabeling() const;

    /// Is the edge labeling "MULTI_TERMINAL".
    bool isMultiTerminal() const;

    /// Is the edge labeling "EV_PLUS".
    bool isEVPlus() const;

    /// Is the edge labeling "INDEX_SET".
    bool isIndexSet() const;

    /// Is the edge labeling "EV_TIMES".
    bool isEVTimes() const;

    /// Check if we match a specific type of forest
    bool matches(bool isR, range_type rt, edge_labeling el) const;

    /// Returns the current policies used by this forest.
    const policies& getPolicies() const;
    policies& getPolicies();

    /// Returns the reduction rule used by this forest.
    policies::reduction_rule getReductionRule() const;

    /// Returns true if the forest is fully reduced.
    bool isFullyReduced() const;

    /// Returns true if the forest is quasi reduced.
    bool isQuasiReduced() const;

    /// Returns true if the forest is identity reduced.
    bool isIdentityReduced() const;
    
     /// Returns true if the forest is user_defined reduced.
    bool isUserDefinedReduced() const;

    /// Returns true if the level is fully reduced.
    bool isFullyReduced(int k) const;
    
    /// Returns true if the level is quasi reduced.
    bool isQuasiReduced(int k) const;
    
    /// Returns true if the level is identity reduced.
    bool isIdentityReduced(int k) const;
    
    int* getLevelReductionRule() const;

    

    inline bool isVarSwap() const {
    	return deflt.swap == policies::variable_swap_type::VAR;
    }
    inline bool isLevelSwap() const {
    	return deflt.swap == policies::variable_swap_type::LEVEL;
    }

    /// Returns the storage mechanism used by this forest.
    node_storage_flags getNodeStorage() const;

    /// Returns the node deletion policy used by this forest.
    policies::node_deletion getNodeDeletion() const;

    /// Are we using pessimistic deletion
    bool isPessimistic() const;

    /// Can we store nodes sparsely
    bool areSparseNodesEnabled() const;

    /// Can we store nodes fully
    bool areFullNodesEnabled() const;

    /// Get forest performance stats.
    const statset& getStats() const;

    /** Get the current number of nodes in the forest, at all levels.
        @return     The current number of nodes, not counting deleted or
                    marked for deletion nodes.
    */
    long getCurrentNumNodes() const;

    /** Get the current total memory used by the forest.
        This should be equal to summing getMemoryUsedForVariable()
        over all variables.
        @return     Current memory used by the forest.
    */
    long getCurrentMemoryUsed() const;

    /** Get the current total memory allocated by the forest.
        This should be equal to summing getMemoryAllocatedForVariable()
        over all variables.
        @return     Current total memory allocated by the forest.
    */
    long getCurrentMemoryAllocated() const;

    /** Get the peak number of nodes in the forest, at all levels.
        This will be at least as large as calling getNumNodes() after
        every operation and maintaining the maximum.
        @return     The peak number of nodes that existed at one time,
                    in the forest.
    */
    long getPeakNumNodes() const;
    
    /** Set the peak number of nodes to the number current number of nodes.
    */
    inline void resetPeakNumNodes() {
      stats.peak_active = stats.active_nodes;
    }

    /** Get the peak memory used by the forest.
        @return     Peak total memory used by the forest.
    */
    long getPeakMemoryUsed() const;
	
	/** Set the peak memory to the current memory.
	*/
	inline void resetPeakMemoryUsed() {
	  stats.peak_memory_used = stats.memory_used;
	}

    /** Get the peak memory allocated by the forest.
        @return     Peak memory allocated by the forest.
    */
    long getPeakMemoryAllocated() const;

    /// Are we about to be deleted?
    bool isMarkedForDeletion() const;

  // ------------------------------------------------------------
  // non-virtual.
  public:
    /// Remove any stale compute table entries associated with this forest.
    void removeStaleComputeTableEntries();

    /// Remove all compute table entries associated with this forest.
    void removeAllComputeTableEntries();

  // ------------------------------------------------------------
  // virtual, with default implementation.
  public:

    /** Create an edge such that
        f(v_1, ..., vh=i, ..., v_n) = terms[i] for 0 <= i < size(vh).

        For example, in a forest with range_type BOOLEAN, with 3 variables,
        all of size 3, if vh == 2. An edge is created such that
        v_1 v_2 v_3 TERM
        X   0   X  terms[0]
        X   1   X  terms[1]
        X   2   X  terms[2]
        where X represents "all possible".

        \a primedLevel is useful only with forests that store relations. Here,
        \a primedLevel is used to indicate whether the edge is to be created
        for the primed or the unprimed level.

        @param  vh    Variable handle.
        @param  vp
                      true: creates node for the primed vh variable.
                      false: creates node for the unprimed vh variable.
        @param  terms Array of boolean terminal values.
        @param  a     return a handle to a node in the forest such that
                      f(v_1, ..., vh=i, ..., v_n) = terms[i]
                      for 0 <= i < size(vh).

        @throws       TYPE_MISMATCH, if the forest's range is not BOOLEAN.
    */
    virtual void createEdgeForVar(int vh, bool vp, const bool* terms, dd_edge& a);

    /** Create an edge such that
        f(v_1, ..., vh=i, ..., v_n) = terms[i] for 0 <= i < size(vh).

        For example, in a forest with range_type INTEGER, with 3 variables,
        all of size 3, if vh == 2. An edge is created such that
        v_1 v_2 v_3 TERM
        X   0   X  terms[0]
        X   1   X  terms[1]
        X   2   X  terms[2]
        where X represents "all possible".

        \a primedLevel is useful only with forests that store relations. Here,
        \a primedLevel is used to indicate whether the edge is to be created
        for the primed or the unprimed level.

        @param  vh    Variable handle.
        @param  vp
                      true: creates node for the primed vh variable.
                      false: creates node for the unprimed vh variable.
        @param  terms Array of boolean terminal values.
        @param  a     return a handle to a node in the forest such that
                      f(v_1, ..., vh=i, ..., v_n) = terms[i]
                      for 0 <= i < size(vh).
        
        @throws       TYPE_MISMATCH, if the forest's range is not INTEGER.
    */
    virtual void createEdgeForVar(int vh, bool vp, const int* terms, dd_edge& a);

    /** Create an edge such that
        f(v_1, ..., vh=i, ..., v_n) = terms[i] for 0 <= i < size(vh).

        For example, in a forest with range_type REAL, with 3 variables,
        all of size 3, if vh == 2. An edge is created such that
        v_1 v_2 v_3 TERM
        X   0   X  terms[0]
        X   1   X  terms[1]
        X   2   X  terms[2]
        where X represents "all possible".

        \a primedLevel is useful only with forests that store relations. Here,
        \a primedLevel is used to indicate whether the edge is to be created
        for the primed or the unprimed level.

        @param  vh    Variable handle.
        @param  vp
                      true: creates node for the primed vh variable.
                      false: creates node for the unprimed vh variable.
        @param  terms Array of boolean terminal values.
        @param  a     return a handle to a node in the forest such that
                      f(v_1, ..., vh=i, ..., v_n) = terms[i]
                      for 0 <= i < size(vh).

        @throws       TYPE_MISMATCH, if the forest's range is not REAL.
    */
    virtual void createEdgeForVar(int vh, bool vp, const float* terms, dd_edge& a);

    /** Create an edge such that
        f(v_1, ..., vh=i, ..., v_n) = i for 0 <= i < size(vh).

        For example, in a forest with range_type INTEGER, with 3 variables,
        all of size 3, if vh == 2. An edge is created such that
        v_1 v_2 v_3 TERM
        X   0   X  0
        X   1   X  1
        X   2   X  2
        where X represents "all possible".

        \a primedLevel is useful only with forests that store relations. Here,
        \a primedLevel is used to indicate whether the edge is to be created
        for the primed or the unprimed level.
  
        @param  vh    Variable handle.
        @param  pr
                      true: creates node for the primed vh variable.
                      false: creates node for the unprimed vh variable.
        @param  a     return a handle to a node in the forest such that
                      f(v_1, ..., vh=i, ..., v_n) = i for 0 <= i < size(vh).
    */
    void createEdgeForVar(int vh, bool pr, dd_edge& a);

    /** Create an edge as the union of several explicit vectors.
        @param  vlist Array of vectors. Each vector has dimension equal
                      to one plus the largest variable handle in the domain.
                      A vector \a x indicates a set of variable assignments,
                      where x[vh] may have a value of DONT_CARE;
                      otherwise x[vh] gives the variable assignment for vh.
        @param  N     Number of vectors (dimension of \a vlist).
        @param  e     returns a handle to a node in the forest, such that
                      f(v_1, ..., v_n) = 1, iff there is a vector
                      x in \a vlist corresponding to the variable assignments
                      v_1, ..., v_n; f(v_1, ..., v_n) = 0, otherwise.

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not BOOLEAN, 
                        or the forest is for relations.
    */
    virtual void createEdge(const int* const* vlist, int N, dd_edge &e);

    /** Create an edge as the union of several vectors and return values.
        @param  vlist Array of vectors. Each vector has dimension equal
                      to one plus the largest variable handle in the domain.
                      A vector \a x indicates a set of variable assignments,
                      where x[vh] may have a value of DONT_CARE;
                      otherwise x[vh] gives the variable assignment for vh.
        @param  terms Array of return values, same dimension as \a vlist.
        @param  N     Number of vectors (dimension of \a vlist).
        @param  e     returns a handle to a node in the forest that encodes
                      the function f: f(v_1, ..., v_n) = terms[j]
                      iff j is the smallest integer such that vector vlist[j]
                      corresponds to the variable assignments v_1, ..., v_n;
                      f(v_1, ..., v_n) = 0, otherwise.

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not INTEGER,
                        or the forest is for relations.
    */
    virtual void createEdge(const int* const* vlist, const int* terms, int N, dd_edge &e);

    /** Create an edge as the union of several vectors and return values.
        @param  vlist Array of vectors. Each vector has dimension equal
                      to one plus the largest variable handle in the domain.
                      A vector \a x indicates a set of variable assignments,
                      where x[vh] may have a value of DONT_CARE;
                      otherwise x[vh] gives the variable assignment for vh.
        @param  terms Array of return values, same dimension as \a vlist.
        @param  N     Number of vectors (dimension of \a vlist).
        @param  e     returns a handle to a node in the forest that encodes
                      the function f: f(v_1, ..., v_n) = terms[j]
                      iff j is the smallest integer such that vector vlist[j]
                      corresponds to the variable assignments v_1, ..., v_n;
                      f(v_1, ..., v_n) = 0, otherwise.

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not REAL,
                        or the forest is for relations.
    */
    virtual void createEdge(const int* const* vlist, const float* terms, int N, dd_edge &e);


    /** Create an edge as the union of several explicit matrices.
        @param  vlist   Array of vectors. Each vector has dimension equal to
                        one plus the largest variable handle in the domain.
                        Vector vlist indicates the set of unprimed variable
                        assignments, and vector vplist indicates the set of
                        primed variable assignments.  Both vlist and vplist
                        may have elements equal to DONT_CARE, and vplist
                        may have elements equal to DONT_CHANGE.
        @param  vplist  Array of vectors, same dimension as \a vlist.
                        A vector \a x = vplist[i] indicates a set of primed
                        variable assignments, where x[vh] less than 0 means
                        "don't change for variable vh" iff we are not
                        changing the unprimed variable, and
                        "don't care for primed variable vh" otherwise.
                        Otherwise x[vh] gives the variable assignment for
                        primed variable vh.
        @param  N       Number of vectors (dimension of \a vlist).
        @param  e       returns a handle to a node in the forest, such that
                        f(v_1, v'_1, ..., v_n, v'_n) = 1, iff there is a
                        vector vlist[i] corresponding to the variable
                        assignments v_1, ..., v_n, and vplist[i] corresponds
                        to variable assignments v'_1, ..., v'_n.
                        f(v_1, v'_1, ..., v_n, v'_n) = 0, otherwise.

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not BOOLEAN, 
                        or the forest is not for relations.
    */
    virtual void createEdge(const int* const* vlist, const int* const* vplist, int N, dd_edge &e);


    /** Create an edge as the union of several explicit matrices.
        @param  vlist   Array of vectors. Each vector has dimension equal to
                        one plus the largest variable handle in the domain.
                        Vector vlist indicates the set of unprimed variable
                        assignments, and vector vplist indicates the set of
                        primed variable assignments.  Both vlist and vplist
                        may have elements equal to DONT_CARE, and vplist
                        may have elements equal to DONT_CHANGE.
        @param  vplist  Array of vectors, same dimension as \a vlist.
                        A vector \a x = vplist[i] indicates a set of primed
                        variable assignments, where x[vh] less than 0 means
                        "don't change for variable vh" iff we are not
                        changing the unprimed variable, and
                        "don't care for primed variable vh" otherwise.
                        Otherwise x[vh] gives the variable assignment for
                        primed variable vh.
        @param  terms   Array of return values, same dimension as \a vlist.
        @param  N       Number of vectors (dimension of \a vlist).
        @param  e       returns a handle to a node in the forest, such that
                        f(v_1, v'_1, ..., v_n, v'_n) = term[i], iff there is
                        a vector vlist[i] corresponding to the variable
                        assignments v_1, ..., v_n, and vplist[i] corresponds
                        to variable assignments v'_1, ..., v'_n.
                        f(v_1, v'_1, ..., v_n, v'_n) = 0, otherwise.

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not INTEGER, 
                        or the forest is not for relations.
    */
    virtual void createEdge(const int* const* vlist, const int* const* vplist, 
        const int* terms, int N, dd_edge &e);


    /** Create an edge as the union of several explicit matrices.
        @param  vlist   Array of vectors. Each vector has dimension equal to
                        one plus the largest variable handle in the domain.
                        Vector vlist indicates the set of unprimed variable
                        assignments, and vector vplist indicates the set of
                        primed variable assignments.  Both vlist and vplist
                        may have elements equal to DONT_CARE, and vplist
                        may have elements equal to DONT_CHANGE.
        @param  vplist  Array of vectors, same dimension as \a vlist.
                        A vector \a x = vplist[i] indicates a set of primed
                        variable assignments, where x[vh] less than 0 means
                        "don't change for variable vh" iff we are not
                        changing the unprimed variable, and
                        "don't care for primed variable vh" otherwise.
                        Otherwise x[vh] gives the variable assignment for
                        primed variable vh.
        @param  terms   Array of return values, same dimension as \a vlist.
        @param  N       Number of vectors (dimension of \a vlist).
        @param  e       returns a handle to a node in the forest, such that
                        f(v_1, v'_1, ..., v_n, v'_n) = term[i], iff there is
                        a vector vlist[i] corresponding to the variable
                        assignments v_1, ..., v_n, and vplist[i] corresponds
                        to variable assignments v'_1, ..., v'_n.
                        f(v_1, v'_1, ..., v_n, v'_n) = 0, otherwise.

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not REAL, 
                        or the forest is not for relations.
    */
    virtual void createEdge(const int* const* vlist, const int* const* vplist, 
        const float* terms, int N, dd_edge &e);


    /** Create an edge for a boolean constant.
        @param  val   Requested constant.
        @param  e     returns a handle to a node in the forest for
                      function f = \a val.

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not BOOLEAN.
    */
    virtual void createEdge(bool val, dd_edge &e);

    /** Create an edge for an integer constant.
        @param  val   Requested constant.
        @param  e     returns a handle to a node in the forest for
                      function f = \a val.

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not BOOLEAN.
    */
    virtual void createEdge(int val, dd_edge &e);

    /** Create an edge for an integer constant.
        @param  val   Requested constant.
        @param  e     returns a handle to a node in the forest for
                      function f = \a val.

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not BOOLEAN.
    */
    virtual void createEdge(float val, dd_edge &e);

    /** Evaluate the function encoded by an edge.
        @param  f     Edge (function) to evaluate.
        @param  vlist List of variable assignments, of dimension one higher
                      than the largest variable handle.
        @param  term  Output parameter, will be set to
                      f(v1 = vlist[v1], ..., vn = vlist[vn]).

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not BOOLEAN, 
                        or the forest is for relations.
    */
    virtual void evaluate(const dd_edge &f, const int* vlist, bool &term)
      const;

    /** Evaluate the function encoded by an edge.
        @param  f     Edge (function) to evaluate.
        @param  vlist List of variable assignments, of dimension one higher
                      than the largest variable handle.
        @param  term  Output parameter, will be set to
                      f(v1 = vlist[v1], ..., vn = vlist[vn]).

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not INTEGER,
                        or the forest is for relations.
    */
    virtual void evaluate(const dd_edge &f, const int* vlist, int &term)
      const;

    /** Evaluate the function encoded by an edge.
        @param  f     Edge (function) to evaluate.
        @param  vlist List of variable assignments, of dimension one higher
                      than the largest variable handle.
        @param  term  Output parameter, will be set to
                      f(v1 = vlist[v1], ..., vn = vlist[vn]).

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not REAL,
                        or the forest is for relations.
    */
    virtual void evaluate(const dd_edge &f, const int* vlist, float &term)
      const;

    /** Evaluate the function encoded by an edge.
        @param  f       Edge (function) to evaluate.
        @param  vlist   List of variable assignments for unprimed variables,
                        of dimension one higher than the largest variable
                        handle.
        @param  vplist  List of variable assignments for primed variables, 
                        of dimension one higher than the largest variable
                        handle.
        @param  term    Output parameter, will be set to
                        f(v1 = vlist[v1], v'1 = vplist[v1], ...,
                        vn = vlist[vn], v'n = vplist[vn]).

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not BOOLEAN, 
                        or the forest is not for relations.
    */
    virtual void evaluate(const dd_edge& f, const int* vlist,
      const int* vplist, bool &term) const;

    /** Evaluate the function encoded by an edge.
        @param  f       Edge (function) to evaluate.
        @param  vlist   List of variable assignments for unprimed variables,
                        of dimension one higher than the largest variable
                        handle.
        @param  vplist  List of variable assignments for primed variables, 
                        of dimension one higher than the largest variable
                        handle.
        @param  term    Output parameter, will be set to
                        f(v1 = vlist[v1], v'1 = vplist[v1], ...,
                        vn = vlist[vn], v'n = vplist[vn]).

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not INTEGER, 
                        or the forest is not for relations.
    */
    virtual void evaluate(const dd_edge& f, const int* vlist,
      const int* vplist, int &term) const;

    /** Evaluate the function encoded by an edge.
        @param  f       Edge (function) to evaluate.
        @param  vlist   List of variable assignments for unprimed variables,
                        of dimension one higher than the largest variable
                        handle.
        @param  vplist  List of variable assignments for primed variables, 
                        of dimension one higher than the largest variable
                        handle.
        @param  term    Output parameter, will be set to
                        f(v1 = vlist[v1], v'1 = vplist[v1], ...,
                        vn = vlist[vn], v'n = vplist[vn]).

        @throws       TYPE_MISMATCH, if
                        the range type of the forest is not REAL, 
                        or the forest is not for relations.
    */
    virtual void evaluate(const dd_edge& f, const int* vlist,
      const int* vplist, float &term) const;

    /** Returns element \a e at index \a i from an Index Set EV+MDD.
    
        size(e) = number of variables in the forest + 1 (for terminals).
        TODO: complete this description

        on return: e[0] will be 1 if the element could be found, 0 otherwise.

        @throws       INVALID_OPERATION, if this is not an Index Set EV+MDD.
    */
    virtual void getElement(const dd_edge& a, int index, int* e);

  // ------------------------------------------------------------
  // abstract virtual.
  public:
    /** Write edges to a file in a format that can be read back later.
        This implies that all nodes "below" those edges are also
        written to the file.
          @param  s   Stream to write to
          @param  E   Array of edges
          @param  n   Dimension of the edge array

          @throws     COULDNT_WRITE, if writing failed
    */
    virtual void writeEdges(output &s, const dd_edge* E, int n) const = 0;

    /** Read edges from a file.
        Allows reconstruction of edges that we
        saved using \a writeEdges().
        The forest does not need to be empty;
        the edges are added to the forest as necessary.
          @param  s   Stream to read from
          @param  E   Array of edges
          @param  n   Dimension of the edge array
          
          @throws     INVALID_FILE, if the file does not match what we expect,
                      including different number of edges specified.
    */
    virtual void readEdges(input &s, dd_edge* E, int n) = 0;

    /** Force garbage collection.
        All disconnected nodes in this forest are discarded along with any
        compute table entries that may include them.
    */
    virtual void garbageCollect() = 0;

    /** Compact the memory for all variables in this forest.
        This is not the same as garbage collection.
    */
    virtual void compactMemory() = 0;

    /** Create an edge representing the subset of a Matrix Diagram.

        size(vlist) == number of variables in the forest + 1 (for terminals)
        size(vlist[i]) == size (i.e. bound) of variable i
        size(vlist) == size(vplist)
        size(vlist[i]) == size(vplist[i])
        If vlist[i][j] is true, that index is included in the mask
        If vlist[i][j] is false, that index is excluded from the mask.
        
        TODO: write a better description (an example might also help).
    */
    /*
    virtual void createSubMatrix(const bool* const* vlist,
      const bool* const* vplist, const dd_edge a, dd_edge& b) = 0;
      */




    /** Display all active (i.e., connected) nodes in the forest.
        This is primarily for aid in debugging.
        @param  strm      Stream to write to.
        @param  verbosity How much information to display.
                          0 : just statistics.
                          1 : all forest nodes + statistics.
                          2 : internal forest + statistics.
    */
    virtual void showInfo(output &strm, int verbosity=0) = 0;


    /** Start logging stats.
          @param  L     Logger to use; if 0, we don't log anything.
                        Will overwrite the old logger.
                        The logger WILL NOT be deleted by the forest
                        (in case we want to log for multiple forests).

          @param  name  Name to use for the forest (for display only).
    */
    void setLogger(logger* L, const char* name);


  // ------------------------------------------------------------
  // For derived classes.
  protected:
    // for debugging:
    void showComputeTable(output &s, int verbLevel) const;

  protected:
    policies deflt;
    statset stats;
    logger *theLogger;

  // ------------------------------------------------------------
  // Ugly details from here down.
  private:  // Defaults
    friend class forest_initializer;
    static policies mddDefaults;
    static policies mxdDefaults;

  private:  // Domain info 
    friend class domain;
    int d_slot;
    domain* d;

  private:  // For operation registration
    friend class operation;

    int* opCount;
    int szOpCount;

    /// Register an operation with this forest.
    /// Called only within operation.
    void registerOperation(const operation* op);

    /// Unregister an operation.
    /// Called only within operation.
    void unregisterOperation(const operation* op);

  private:  // For edge registration
    friend class dd_edge;

    // structure to store references to registered dd_edges.
    struct edge_data {
      // If nextHole >= 0, this is a hole, in which case nextHole also
      // refers to the array index of the next hole in edge[]
      int nextHole;
      // registered dd_edge
      dd_edge* edge;
    };

    // Array of registered dd_edges
    edge_data *edge;

    // Size of edge[]
    unsigned sz;
    // Array index: 1 + (last used slot in edge[])
    unsigned firstFree;
    // Array index: most recently created hole in edge[]
    int firstHole;

    void registerEdge(dd_edge& e);
    void unregisterEdge(dd_edge& e);
    void unregisterDDEdges();

  protected:
    class edge_visitor {
      public:
        edge_visitor();
        virtual ~edge_visitor();
        virtual void visit(dd_edge &e) = 0;
    };
    void visitRegisteredEdges(edge_visitor &ev);

  private:
    bool isRelation;
    bool is_marked_for_deletion;
    range_type rangeType;
    edge_labeling edgeLabel;

    /// Mark for deletion.
    void markForDeletion();

    friend void MEDDLY::destroyForest(MEDDLY::forest* &f);

    // our ID
    unsigned fid;

    // global one
    static unsigned gfid;
};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                         variable class                         *
// *                                                                *
// *                                                                *
// ******************************************************************

/** Variable class.
    Abstract base class.
    A variable consists of an optional name, and a bound.
    A single variable object is used to describe both 
    the primed and unprimed versions of the variable.

    Note: variables are automatically deleted when
    removed from their last domain.

    Additional features are provided in the expert interface.
*/
class MEDDLY::variable {
  protected:
    variable(int bound, char* name);
  protected:
    virtual ~variable();
  public:
    int getBound(bool primed) const;
    const char* getName() const;
    void setName(char* newname);
  protected:
    int un_bound;
    int pr_bound;
  private:
    char* name;
};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                      variable order class                      *
// *                                                                *
// *                                                                *
// ******************************************************************

class MEDDLY::variable_order {
  protected:
    std::vector<int> level2var;
    std::vector<int> var2level;

  public:
    variable_order(const int* order, int size);
    variable_order(const variable_order& order);

    int getVarByLevel(int level) const;
    int getLevelByVar(int var) const;

    // Exchange two variables
    // The two variables don't have to be adjacent
    void exchange(int var1, int var2);

    bool is_compatible_with(const int* order) const;
    bool is_compatible_with(const variable_order& order) const;
};


// ******************************************************************
// *                                                                *
// *                                                                *
// *                          domain class                          *
// *                                                                *
// *                                                                *
// ******************************************************************


/** Domain class.
    Abstract base class.
    A domain is an ordered collection of variables,
    along with a rich set of operations for adding and removing variables.
    A variable may be shared in more than one domain
    (see the expert interface on how to do this safely).
  
    When a domain is destroyed, all of its forests are destroyed.
*/
class MEDDLY::domain {
  public:
    static const int TERMINALS;
  public:
    /** Create all variables at once, from the bottom up.
        Requires the domain to be "empty" (containing no variables or
        forests).  
  
        @param  bounds  variable bounds.
                        bounds[i] gives the bound for the variable
                        at level i+1.
        @param  N       Number of variables.
    */
    virtual void createVariablesBottomUp(const int* bounds, int N) = 0;

    /** Create a forest in this domain.
        Conceptually, a forest is a structure used to represent a
        collection of functions over a common domain. For simplicity
        (although it is a slight abuse of notation) a forest may represent
        "vectors" or "sets" over a domain, or "matrices" or "relations".
        In case of matrices / relations, the forest uses primed and unprimed
        versions of every variable in the domain. 
  
        @param  rel     Is this a relation / matrix, versus a set / vector.
        @param  t       Range type of the functions, namely,
                        booleans, integers, or reals.
        @param  ev      Edge labeling mechanism, i.e., should this be a
                        Multi-terminal decision diagram forest,
                        edge-valued with plus/times decision diagram forest.
        @param  p       Policies to use within the forest.
        @param  tv      Transparent value.
        @return 0       if an error occurs, a new forest otherwise.
    */
    forest* createForest(bool rel, forest::range_type t,
      forest::edge_labeling ev, const forest::policies &p,int* level_reduction_rule=NULL, int tv=0);

    /// Create a forest using the library default policies.
    forest* createForest(bool rel, forest::range_type t,
      forest::edge_labeling ev);

    /// Get the number of variables in this domain.
    int getNumVariables() const;

    /** Get the specified bound of a variable.
        No range checking, for speed.
        @param  lev     Level number, should be 1 for bottom-most
                        and getNumVariables() for top-most.
        @param  prime   If prime is true, get the bound for 
                        the primed variable.
        @return         The bound set for variable at level \a lev.
    */
    int getVariableBound(int lev, bool prime = false) const;

    /// @return The variable at level \a lev.
    const variable* getVar(int lev) const;
    /// @return The variable at level \a lev.
    variable* useVar(int lev);

    /** Write the domain to a file in a format that can be read back later.
          @param  s   Stream to write to

          @throws     COULDNT_WRITE, if writing failed
    */
    virtual void write(output &s) const = 0;

    /** Initialize the domain from data in a file.
        Allows reconstruction of a domain that 
        we saved using \a write().
        The domain should be empty.
          @param  s   Stream to read from
          
          @throws     INVALID_FILE, if the file does not match what we expect 
    */
    virtual void read(input &s) = 0;

    /** Display lots of information about the domain.
        This is primarily for aid in debugging.
        @param  strm    Stream to write to.
    */
    void showInfo(output &strm);

    /// Free the slot that the forest is using.
    void unlinkForest(forest* f, int slot);

    // --------------------------------------------------------------------

  protected:
    /// Constructor.
    domain(variable** v, int N);

    /// Destructor.
    virtual ~domain();

    variable** vars;
    int nVars;

    // var_orders[0] is reserved to store the default variable order
    std::vector< std::shared_ptr<const variable_order> > var_orders;
    std::shared_ptr<const variable_order> default_var_order;

  private:
    bool is_marked_for_deletion;
    forest** forests;
    int szForests;

    /// Find a free slot for a new forest.
    int findEmptyForestSlot();

    /// Mark this domain for deletion
    void markForDeletion();

    friend void MEDDLY::destroyDomain(domain* &d);
    friend void MEDDLY::cleanup();

  public:
    bool hasForests() const;
    bool isMarkedForDeletion() const;

    std::shared_ptr<const variable_order> makeVariableOrder(const int* order);
    std::shared_ptr<const variable_order> makeVariableOrder(const variable_order& order);
    std::shared_ptr<const variable_order> makeDefaultVariableOrder();
    void cleanVariableOrders();

  private:
    /// List of all domains; initialized in meddly.cc
    static domain** dom_list;
    /// List of free slots (same dimension as dom_list); c.f. meddly.cc
    static int* dom_free;
    /// Size of domain list; initialized in meddly.cc
    static int dom_list_size;
    /// First free slot; initialized in meddly.cc
    static int free_list;
    /// Index of this domain in the domain list.
    int my_index;

    static void expandDomList();
    static void markDomList();
    static void deleteDomList();

  public:
    int ID() const;
};


// ******************************************************************
// *                                                                *
// *                                                                *
// *                         dd_edge  class                         *
// *                                                                *
// *                                                                *
// ******************************************************************

/** Structure for handles to edges in forests.

    A dd_edge is a handle for user manipulation of functions stored in
    forests.

    There are a few useful operations that can be applied directly
    to a dd_edge; all the rest are done either through the "parent" forest,
    or through operations.  These include:
   
    - Deletion of a dd_edge.  This will cause the parent forest to recycle
      nodes as appropriate.
    
    - Checking for equality of two dd_edges, using the method equals().
*/
class MEDDLY::dd_edge {
  public:
    /// Empty constructor.
    dd_edge();

    /** Constructor.
        Creates an empty edge in forest \a p.
        @param  p     forest to which this dd_edge will belong to.
    */
    dd_edge(forest* p);

    /** Copy Constructor.
        @param  e       dd_edge to copy.
    */
    dd_edge(const dd_edge &e);

    /** Assignment operator.
        @param  e       dd_edge to copy.
        @return         the new dd_edge.
    */
    dd_edge& operator=(const dd_edge &e);

    /// Destructor.  Will notify parent as appropriate.
    ~dd_edge();

  private:
    void init(const dd_edge &e);
    void destroy();


  public:

    /** Clears the contents of this edge. It will belong to the same
        forest as before.
    */
    void clear();

    /** Obtain a modifiable copy of the forest owning this edge.
        @return         the forest to which this edge belongs to.
    */
    forest* getForest() const;

    /** Get this dd_edge's node handle.
        @return         the node handle.
    */
    node_handle getNode() const;

    /// Get this dd_edge's edge value (only valid for edge-valued MDDs).
    void getEdgeValue(int& ev) const;

    /// Get this dd_edge's edge value (only valid for edge-valued MDDs).
    void getEdgeValue(float& ev) const;

    /** Get this dd_edge's level.
        @return         the level.
    */
    int getLevel() const;

    /** Get node cardinality.
        Provided for backward compatibility.
        Use apply(CARDINALITY, ...) instead.
        @return         the cardinality of the node.
    */
    double getCardinality() const;

    /** Counts the number of unique nodes in this decision diagram.
        @return       the number of unique nodes starting at the root node
                      of this dd_edge.
    */
    unsigned getNodeCount() const;

    /** Counts the number of unique edges in this decision diagram.
        @param  countZeroes
                      if true, the stored zero edges are also counted
                      (sparse nodes do not store zero edges, so this
                      does not effect them; truncated nodes do store
                      some zero edges, so those edges will be counted).
        @return       the number of unique edges starting at the root node
                      of this dd_edge.
    */
    unsigned getEdgeCount(bool countZeroes = false) const;

    /** Modifies the dd_edge fields.
        The dd_edge is cleared (it will still belong to the same forest),
        and the dd_edge data is filled with the data provided.
        @param  node    node handle.
    */
    void set(node_handle node);

    /** Modifies the dd_edge fields.
        The dd_edge is cleared (it will still belong to the same forest),
        and the dd_edge data is filled with the data provided.
        @param  node    node handle.
        @param  value   value of edge coming into the node (only useful
                        for edge-valued MDDs)
    */
    void set(node_handle node, int value);

    /** Modifies the dd_edge fields.
        The dd_edge is cleared (it will still belong to the same forest),
        and the dd_edge data is filled with the data provided.
        @param  node    node handle.
        @param  value   value of edge coming into the node (only useful
                        for edge-valued MDDs)
    */
    void set(node_handle node, float value);

    /** Check for equality.
        @return true    iff this edge has the same parent and refers to
                        the same edge as \a e.
    */
    bool operator==(const dd_edge& e) const;

    /** Check for inequality.
        @return true    iff this edge does not refer to the same edge as \a e.
    */
    bool operator!=(const dd_edge& e) const;

    /** Plus operator.
        BOOLEAN forests: Union; INTEGER/REAL forests: Addition.
        @param  e       dd_edge to Union/Add with this dd_edge.
        @return         \a this + \a e.
    */
    const dd_edge operator+(const dd_edge& e) const;

    /** Compound Plus operator.
        BOOLEAN forests: Union; INTEGER/REAL forests: Addition.
        This edge is overwritten with the result of the operation.
        @param  e       dd_edge to Union/Add with this dd_edge.
        @return         \a this + \a e.
    */
    dd_edge& operator+=(const dd_edge &e);

    /** Star operator.
        BOOLEAN forests: Intersection; INTEGER/REAL forests: Multiplication.
        @param  e       dd_edge to Intersection/Multiply with this dd_edge.
        @return         \a this * \a e.
    */
    const dd_edge operator*(const dd_edge& e) const;

    /** Compound Star operator.
        BOOLEAN forests: Intersection; INTEGER/REAL forests: Multiplication.
        This edge is overwritten with the result of the operation.
        @param  e       dd_edge to Intersection/Multiply with this dd_edge.
        @return         \a this * \a e.
    */
    dd_edge& operator*=(const dd_edge &e);

    /** Minus operator.
        BOOLEAN forests: Difference; INTEGER/REAL forests: Subtraction.
        @param  e       dd_edge for difference/subtract.
        @return         \a this - \a e.
    */
    const dd_edge operator-(const dd_edge& e) const;

    /** Compound Minus operator.
        BOOLEAN forests: Difference; INTEGER/REAL forests: Subtraction.
        This edge is overwritten with the result of the operation.
        @param  e       dd_edge for difference/subtract.
        @return         \a this - \a e.
    */
    dd_edge& operator-=(const dd_edge &e);

    /** Divide operator.
        BOOLEAN forests: INVALID; INTEGER/REAL forests: Division.
        @param  e       dd_edge for division.
        @return         \a this / \a e.
    */
    const dd_edge operator/(const dd_edge& e) const;

    /** Compound Divide operator.
        BOOLEAN forests: INVALID; INTEGER/REAL forests: Division.
        This edge is overwritten with the result of the operation.
        @param  e       dd_edge for division.
        @return         \a this / \a e.
    */
    dd_edge& operator/=(const dd_edge &e);

    /** Display the edge information.
        This is primarily for aid in debugging.
        Note that cardinality only works for MDDs, MTMDDs, MXDs and MTMXDs.
        @param  strm      File stream to write to.
        @param  verbosity 0: default
                          1: default + cardinality
                          2: default + displays graph rooted at this node.
                          3: default + cardinality + graph.
    */
    void show(output &s, int verbosity = 0) const;

    /// Write to a file
    void write(output &s, const node_handle* map) const;

    /// Read from a file
    void read(forest* p, input &s, const node_handle* map);

  private:
    friend class forest;

    void setIndex(int ind);
    int getIndex() const;

    forest *parent;
    int index;  //  our slot number in the parent forest's list

    node_handle node;
    node_handle raw_value;

    binary_operation* opPlus;
    binary_operation* opStar;
    binary_operation* opMinus;
    binary_operation* opDivide;

    // called when the parent is destroyed
    void orphan();
};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                        enumerator class                        *
// *                                                                *
// *                                                                *
// ******************************************************************

/** Class for enumerating values encoded by a dd-edge.
    Effectively, a single class encapsulating 
    several possible iterators.
*/
class MEDDLY::enumerator {
  public:
    class iterator {
      public:
        iterator(const expert_forest* F);
        virtual ~iterator();

        bool build_error() const;

        virtual bool start(const dd_edge &e);
        virtual bool start(const dd_edge &e, const int* m);
        virtual bool next() = 0;

        /**
            Return the highest level changed during the last increment.
        */
        int levelChanged() const;

        /** Get the current variable assignments.
            For variable i, use index i for the
            unprimed variable, and index -i for the primed variable.
        */
        const int* getAssignments() const;

        /** Get primed assignments.
            It is much faster to use getAssigments()
            and look at the negative indexes;
            however, this works.
        */
        const int* getPrimedAssignments();

        /// For integer-ranged edges, get the current non-zero value.
        virtual void getValue(int& edgeValue) const;
    
        /// For real-ranged edges, get the current non-zero value.
        virtual void getValue(float& edgeValue) const;
        

      protected:
        // Current parent forest.
        const expert_forest* F;
        // Path, as list of unpacked nodes
        unpacked_node*    rawpath;
        unpacked_node*    path;   // rawpath, shifted so we can use path[-k]
        // Path nnz pointers
        int*      rawnzp;
        int*      nzp;   // rawnzp, shifted so we can use nzp[-k]
        // Path indexes
        int*      rawindex;
        int*      index;  // rawindex, shifted so we can use index[-k]
        // 
        int       minLevel; // 1 or -#vars, depending.
        int       maxLevel; // #vars
        //
        int       level_change; 

      private:
        // Used only by getPrimedAssignments.
        int*      prindex;
    };

  public:
    /// Enumerator type.
    enum type {
      EMPTY,
      FULL,       
      ROW_FIXED,
      COL_FIXED
    };

  public:
    /// Empty constructor.
    enumerator();

    /// Proper constructor.
    enumerator(type t, const forest* F);

    /** What is usually wanted constructor.
        Equivalent to enumerator(FULL, e.getForest())
        followed by start(e).
    */
    enumerator(const dd_edge &e);

    /// Destructor.
    ~enumerator();

    /// Re-initialize
    void init(type t, const forest* F);

    operator bool() const;

    void operator++();

    /** 
        Start iterating through edge e.
    */
    void start(const dd_edge &e);

    /** Start iterating through edge e.
        The unprimed variables will be fixed to
        the given values.
          @param  e         Edge to iterate.
                            Must be a relation.
          @param  minterm   Array of dimension 1+vars in e.
                            minterm[k] gives the fixed variable
                            assignment for (unprimed) variable k.
    */
    void startFixedRow(const dd_edge &e, const int* minterm);

    /** Start iterating through edge e.
        The primed variables will be fixed to
        the given values.
          @param  e         Edge to iterate.
                            Must be a relation.
          @param  minterm   Array of dimension 1+vars in e.
                            minterm[k] gives the fixed variable
                            assignment for (unprimed) variable k.
    */
    void startFixedColumn(const dd_edge &e, const int* minterm);


    /** Get the current variable assignments.
        For variable i, use index i for the
        unprimed variable, and index -i for the primed variable.
    */
    const int* getAssignments() const;

    /// Get the current primed variable assignments.
    const int* getPrimedAssignments() const;

    void getValue(int &v) const;
    void getValue(float &v) const;

    int levelChanged() const;

    type getType() const;

  private:
    iterator* I;
    bool is_valid;
    type T;
};

#include "meddly.hh"
#endif
