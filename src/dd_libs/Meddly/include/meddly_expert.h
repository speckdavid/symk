
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


/*! \file meddly_expert.h

    Low-level MDD library interface.

    This interface is for "expert" users who want to define new
    operations, or for library developers to define the built-in operations.
    Casual users probably only need the interface provided by "meddly.h".

    The first part of the interface describes the expert interface and the
    second part contains implementations of virtual functions in the interface.

    IMPORTANT: meddly.h must be included before including this file.
    TODO: Operations are not thread-safe.
*/

#ifndef MEDDLY_EXPERT_H
#define MEDDLY_EXPERT_H

#include <string.h>
#include <unordered_map>
#include <vector>
#include <cstdint>

// Flags for development version only. Significant reduction in performance.
#ifdef DEVELOPMENT_CODE
#define RANGE_CHECK_ON
#define DCASSERTS_ON
#endif


#define INLINED_COUNT
#define INLINED_NEXT

// #define TRACK_DELETIONS
// #define TRACK_CACHECOUNT

// Use this for assertions that will fail only when your
// code is wrong.  Handy for debugging.
#ifdef DCASSERTS_ON
#define MEDDLY_DCASSERT(X) assert(X)
#else
#define MEDDLY_DCASSERT(X)
#endif

// Use this for range checking assertions that should succeed.
#ifdef RANGE_CHECK_ON
#define MEDDLY_CHECK_RANGE(MIN, VALUE, MAX) { assert(VALUE < MAX); assert(VALUE >= MIN); }
#else
#define MEDDLY_CHECK_RANGE(MIN, VALUE, MAX)
#endif


namespace MEDDLY {

  // classes defined here

  class expert_variable;
  class expert_domain;

  // wrapper for temporary nodes
  class unpacked_node;  // replacement for node_reader, node_builder

  // EXPERIMENTAL - matrix wrappers for unprimed, primed pairs of nodes
  class unpacked_matrix;

  /*
  
    class op_initializer;

    Generalized to class initializer_list.
  */

  class initializer_list;

  /*
    class cleanup_procedure;

    Subsumed by class initializer_list.
  */

  // Actual node storage
  class node_storage_style;
  class node_storage;

  class expert_forest;

  class opname;
  class unary_opname;
  class binary_opname;
  class specialized_opname;
  class numerical_opname;
  class satpregen_opname;
  class satotf_opname;

  class ct_initializer;
  class compute_table_style;
  class compute_table;

  class operation;
  class unary_operation;
  class binary_operation;
  class specialized_operation;

  class global_rebuilder;

  // classes defined elsewhere
  class base_table;
  class unique_table;

  class reordering_base;

  // ******************************************************************
  // *                                                                *
  // *                   Named numerical operations                   *
  // *                                                                *
  // ******************************************************************

  /** Computes y = y + xA.
      x and y are vectors, stored explicitly, and A is a matrix.
      x_ind and y_ind specify how minterms are mapped to indexes
      for vectors x and y, respectively.
  */
  extern const numerical_opname* EXPLVECT_MATR_MULT;
  // extern const numerical_opname* VECT_MATR_MULT; // renamed!

  /** Computes y = y + Ax.
      x and y are vectors, stored explicitly, and A is a matrix.
      x_ind and y_ind specify how minterms are mapped to indexes
      for vectors x and y, respectively.
  */
  extern const numerical_opname* MATR_EXPLVECT_MULT;
  // extern const numerical_opname* MATR_VECT_MULT; // renamed!

  // ******************************************************************
  // *                                                                *
  // *                  Named saturation operations                   *
  // *                                                                *
  // ******************************************************************

  /** Forward reachability using saturation.
      Transition relation is already known.
  */
  extern const satpregen_opname* SATURATION_FORWARD;

  /** Backward reachability using saturation.
      Transition relation is already known.
  */
  extern const satpregen_opname* SATURATION_BACKWARD;

  /** Forward reachability using saturation.
      Transition relation is not completely known,
      will be built along with reachability set.
  */
  extern const satotf_opname* SATURATION_OTF_FORWARD;

  // ******************************************************************
  // *                                                                *
  // *                      Operation management                      *
  // *                                                                *
  // ******************************************************************

  /// Remove an existing operation from the operation cache.
  void removeOperationFromCache(operation* );

  /** Find, or build if necessary, a unary operation.
        @param  code    Operation we want
        @param  arg     Argument forest
        @param  res     Result forest
        @return         The matching operation, if it already exists;
                        a new operation, otherwise.
  */
  unary_operation* getOperation(const unary_opname* code,
    expert_forest* arg, expert_forest* res);

  /** Find, or build if necessary, a unary operation.
        @param  code    Operation we want
        @param  arg     Argument forest from this dd_edge
        @param  res     Result forest from this dd_edge
        @return         The matching operation, if it already exists;
                        a new operation, otherwise.
  */
  unary_operation* getOperation(const unary_opname* code,
    const dd_edge& arg, const dd_edge& res);

  /** Find, or build if necessary, a unary operation.
        @param  code    Operation we want
        @param  arg     Argument forest
        @param  res     Result type
        @return         The matching operation, if it already exists;
                        a new operation, otherwise.
  */
  unary_operation* getOperation(const unary_opname* code,
    expert_forest* arg, opnd_type result);

  /** Find, or build if necessary, a unary operation.
        @param  code    Operation we want
        @param  arg     Argument forest from this dd_edge
        @param  res     Result type
        @return         The matching operation, if it already exists;
                        a new operation, otherwise.
  */
  unary_operation* getOperation(const unary_opname* code,
    const dd_edge& arg, opnd_type result);


  /** Find, or build if necessary, a binary operation.
        @param  code    Operation we want
        @param  arg1    Argument 1 forest
        @param  arg2    Argument 2 forest
        @param  res     Result forest
        @return         The matching operation, if it already exists;
                        a new operation, otherwise.
  */
  binary_operation* getOperation(const binary_opname* code,
    expert_forest* arg1, expert_forest* arg2, expert_forest* res);

  /** Find, or build if necessary, a binary operation.
        @param  code    Operation we want
        @param  arg1    Argument 1 forest taken from this dd_edge
        @param  arg2    Argument 2 forest taken from this dd_edge
        @param  res     Result forest taken from this dd_edge
        @return         The matching operation, if it already exists;
                        a new operation, otherwise.
  */
  binary_operation* getOperation(const binary_opname* code,
    const dd_edge& arg1, const dd_edge& arg2, const dd_edge& res);

  /** Safely destroy the given unary operation. 
      It should be unnecessary to call this directly.
  */
  void destroyOperation(unary_operation* &op);

  /** Safely destroy the given binary operation. 
      It should be unnecessary to call this directly.
  */
  void destroyOperation(binary_operation* &op);

  /// Safely destroy the given numerical operation.
  void destroyOperation(specialized_operation* &op);

  /// Should not be called directly.
  void destroyOpInternal(operation* op);

  // ******************************************************************
  // *                                                                *
  // *                  library management functions                  *
  // *                                                                *
  // ******************************************************************

  /*
  /// Builds an initializer for MEDDLY's builtin operations.
  op_initializer* makeBuiltinInitializer();

    Use defaultInitializerList() instead
  */

  /**
    Build list of initializers for Meddly.
    Custom-built initialization lists will usually include this list.
      @param    prev    Initializers to execute before the default list;
                        can be null.

      @return   List of initializers.
  */
  initializer_list* defaultInitializerList(initializer_list* prev);


  /** Initialize the library with custom settings.
      Should be called before using any other functions.
        @param  L   List of initializers.  Will execute the "setup()"
                    methods in order now, and the "cleanup()" methods
                    in reverse order on library cleanup.
  */
  void initialize(initializer_list* L);


} // namespace MEDDLY



// ******************************************************************
// *                                                                *
// *                     expert_variable  class                     *
// *                                                                *
// ******************************************************************

class MEDDLY::expert_variable : public variable {
  public:
    expert_variable(int b, char* n);

    /// Update our list of domains: add \a d.
    void addToList(domain* d);
    /// Update our list of domains: remove \a d.
    void removeFromList(const domain* d);

    /** Enlarge the possible values for a variable.
      This could modify all nodes in all forests, depending on the
      choice of reduction rule.
      @param  prime   If prime is true, enlarge the bound for
                      the primed variable only, otherwise both
                      the primed and unprimed are enlarged.
      @param  b       New bound, if less than the current bound
                      an error is thrown.
    */
    void enlargeBound(bool prime, int b);

    /** Shrink the possible values for a variable.
      This could modify all nodes in all forests, depending on the
      choice of reduction rule.
      @param  b       New bound, if more than the current bound
                      an error is thrown.
      @param  force   If \a b is too small, and information will be lost,
                      proceed anyway if \a force is true, otherwise
                      return an error code.
    */
    void shrinkBound(int b, bool force);

  private:
    domain** domlist;
    int dl_alloc;
    int dl_used;

    virtual ~expert_variable();
};


// ******************************************************************
// *                                                                *
// *                      expert_domain  class                      *
// *                                                                *
// ******************************************************************

class MEDDLY::expert_domain : public domain {
  public:
    expert_domain(variable**, int);

    virtual void createVariablesBottomUp(const int* bounds, int N);

    /** Create all variables at once, from the top down.
      Requires the domain to be "empty" (containing no variables or forests).
      @param  bounds  Current variable bounds.
                      bounds[0] gives the bound for the top-most variable,
                      and bounds[N-1] gives the bound for the bottom-most
                      variable.
      @param  N       Number of variables.
    */
    void createVariablesTopDown(const int* bounds, int N);

    /** Insert a new variable.
          @param  lev   Level to insert above; use 0 for a
                        new bottom-most level.
          @param  v     Variable to insert.
    */
    void insertVariableAboveLevel(int lev, variable* v);

    /** Remove a variable at the specified level.
        An error is thrown if the variable size is not 1.
        Use shrinkVariableBound() to make the bound 1.
        All forests are modified as appropriate.
          @param  lev   Level number.
    */
    void removeVariableAtLevel(int lev);

    /** Find the level of a given variable.
          @param  v   Variable to search for.
          @return 0, if the variable was not found;
                  i, with getVar(i) == v, otherwise.
    */
    int findLevelOfVariable(const variable* v) const;

    expert_variable* getExpertVar(int lev) const;
    const expert_variable* readExpertVar(int lev) const;

    /** Add a new variable with bound 1.
      Can be used when the domain already has forests, in which case
      all forests are modified as appropriate.
      @param  below   Placement information: the new variable will appear
                      immediately above the level \a below.
    */
    void createVariable(int below);


    /** Swap the locations of variables in forests.
      I.e., changes the variable ordering of all forests with this domain.
      @param  lev1    Level of first variable.
      @param  lev2    Level of second variable.
    */
    void swapOrderOfVariables(int lev1, int lev2);

    /** Find the actual bound of a variable.
      @param  vh      Variable handle.
      @return         The smallest shrinkable bound before information loss
                      for variable \a vh. If \a vh is invalid, or TERMINALS,
                      returns 0.
    */
    int findVariableBound(int vh) const;

    /** Enlarge the possible values for a variable.
      This could modify all nodes in all forests, depending on the
      choice of reduction rule.
      @param  vh     Variable handle.
      @param  prime   If prime is true, enlarge the bound for
                      the primed variable only, otherwise both
                      the primed and unprimed are enlarged.
      @param  b       New bound, if less than the current bound
                      an error code is returned.
    */
    void enlargeVariableBound(int vh, bool prime, int b);

    /** Shrink the possible values for a variable.
      This could modify all nodes in all forests, depending on the
      choice of reduction rule.
      @param  lev     Variable handle.
      @param  b       New bound, if more than the current bound
                      an error code is returned.
      @param  force   If \a b is too small, and information will be lost,
                      proceed anyway if \a force is true, otherwise
                      return an error code.
    */
    void shrinkVariableBound(int vh, int b, bool force);

    virtual void write(output &s) const;
    virtual void read(input &s);

  protected:
    ~expert_domain();
};


// ******************************************************************
// *                                                                *
// *                      unpacked_node  class                      *
// *                                                                *
// ******************************************************************

/** Class for reading nodes.
    Ideally - used anywhere we want to read node data.
    Backend implementation may change :^)
    Implemented in node_wrappers.cc.
    Readers may be "full" or "sparse",
    regardless of how the actual node is stored.

    Currently, a node is:
      array of node_handles, for the downward pointers
      array of integers, for indexes (sparse only)
      "compact" chunk of memory for edge values
*/
class MEDDLY::unpacked_node { 
  public:
    /**
        Options for filling an unpacked node from an existing one.
    */
    enum storage_style {
      /// Unpacked node should be stored as truncated full
      FULL_NODE,
      /// Unpacked node should be stored sparsely
      SPARSE_NODE,
      /// Unpacked node should be stored same as packed node
      AS_STORED
    };

  public:
    /** Constructor.
     The class must be "filled" by a forest before
     it can be used, however.
     */
    unpacked_node();

    /// Destructor.
    ~unpacked_node();

    /// Free memory, but don't delete.
    void clear();

  public:
  /* Initialization methods, primarily for reading */

    void initFromNode(const expert_forest *f, node_handle node, bool full);
    void initFromNode(const expert_forest *f, node_handle node, storage_style st2);

    void initRedundant(const expert_forest *f, int k, node_handle node, bool full);
    void initRedundant(const expert_forest *f, int k, int ev, node_handle node, bool full);
    void initRedundant(const expert_forest *f, int k, float ev, node_handle node, bool full);

    void initIdentity(const expert_forest *f, int k, int i, node_handle node, bool full);
    void initIdentity(const expert_forest *f, int k, int i, int ev, node_handle node, bool full);
    void initIdentity(const expert_forest *f, int k, int i, float ev, node_handle node, bool full);

  /* Create blank node, primarily for writing */
    
    void initFull(const expert_forest *f, int level, int tsz); 
    void initSparse(const expert_forest *f, int level, int nnz);

  public:
  /* For convenience: get recycled instance and initialize */

    static unpacked_node* newFromNode(const expert_forest *f, node_handle node, bool full);
    static unpacked_node* newFromNode(const expert_forest *f, node_handle node, storage_style st2);

    static unpacked_node* newRedundant(const expert_forest *f, int k, node_handle node, bool full);
    static unpacked_node* newRedundant(const expert_forest *f, int k, int ev, node_handle node, bool full);
    static unpacked_node* newRedundant(const expert_forest *f, int k, float ev, node_handle node, bool full);

    static unpacked_node* newIdentity(const expert_forest *f, int k, int i, node_handle node, bool full);
    static unpacked_node* newIdentity(const expert_forest *f, int k, int i, int ev, node_handle node, bool full);
    static unpacked_node* newIdentity(const expert_forest *f, int k, int i, float ev, node_handle node, bool full);

    static unpacked_node* newFull(const expert_forest *f, int level, int tsz);
    static unpacked_node* newSparse(const expert_forest *f, int level, int nnz);

  public:
  /* Display / access methods */

    /** Write a node in human-readable format.

        @param  s       Output stream.
        @param  details Should we show "details" or not.
    */
    void show(output &s, bool details) const;

    /** Write a node in machine-readable format.

        @param  s       Output stream.
        @param  map     Translation to use on node handles.
                        Allows us to renumber nodes as we write them.
    */
    void write(output &s, const node_handle* map) const;

    /// Get a pointer to the unhashed header data.
    const void* UHptr() const;

    /// Modify a pointer to the unhashed header data
    void* UHdata();

    /// Get the number of bytes of unhashed header data.
    int UHbytes() const;

    /// Get a pointer to the hashed header data.
    const void* HHptr() const;

    /// Modify a pointer to the hashed header data.
    void* HHdata();

    /// Get the number of bytes of hashed header data.
    int HHbytes() const;

    /** Get a downward pointer.
          @param  n   Which pointer.
          @return     If this is a full reader,
                      return pointer with index n.
                      If this is a sparse reader,
                      return the nth non-zero pointer.
    */
    node_handle d(int n) const;

    /** Reference to a downward pointer.
          @param  n   Which pointer.
          @return     If this is a full reader,
                      modify pointer with index n.
                      If this is a sparse reader,
                      modify the nth non-zero pointer.
    */
    node_handle& d_ref(int n);

    /** Get the index of the nth non-zero pointer.
        Use only for sparse readers.
     */
    int i(int n) const;

    /** Modify the index of the nth non-zero pointer.
        Use only for sparse readers.
    */
    int& i_ref(int n);

    /// Get a pointer to an edge
    const void* eptr(int i) const;

    /// Modify pointer to an edge
    void* eptr_write(int i);

    /// Get the edge value, as an integer.
    void getEdge(int i, int& ev) const;

    /// Get the edge value, as a float.
    void getEdge(int i, float& ev) const;

    /// Set the edge value, as an integer.
    void setEdge(int i, int ev);

    /// Set the edge value, as a float.
    void setEdge(int i, float ev);

    /// Get the edge value, as an integer.
    int ei(int i) const;

    /// Get the edge value, as a float.
    float ef(int i) const;

    /// Get the level number of this node.
    int getLevel() const;

    /// Set the level number of this node.
    void setLevel(int k);

    /// Get the size of this node (full readers only).
    int getSize() const;

    /// Get the number of nonzeroes of this node (sparse readers only).
    int getNNZs() const;

    /// Is this a sparse reader?
    bool isSparse() const;

    /// Is this a full reader?
    bool isFull() const;

    /// Does this node have edge values?
    bool hasEdges() const;

    /// Number of bytes per edge
    int edgeBytes() const;

    // For debugging unique table.
    unsigned hash() const;

    void setHash(unsigned H);

    void computeHash();

  public:

    /// Change the size of a node
    void resize(int ns);

    /// Shrink the size of a (truncated) full node
    void shrinkFull(int ns);

    /// Shrink the size of a sparse node
    void shrinkSparse(int ns);

    /// Called within expert_forest to allocate space.
    ///   @param  p     Parent.
    ///   @param  k     Level number.
    ///   @param  ns    Size of node.
    ///   @param  full  If true, we'll be filling a full reader.
    ///                 Otherwise it is a sparse one.
    void bind_to_forest(const expert_forest* p, int k, int ns, bool full);

    /// Called by node_storage when building an unpacked
    /// node based on how it's stored.
    void bind_as_full(bool full);

  public:
    // Centralized recycling
    static unpacked_node* useUnpackedNode();
    static void recycle(unpacked_node* r);
    static void freeRecycled();


  private:
    const expert_forest* parent;
    static unpacked_node* freeList;
    unpacked_node* next; // for recycled list
    /*
      TBD - extra info that is not hashed
    */
    void* extra_unhashed;
    int ext_uh_alloc;
    int ext_uh_size;
    /*
     Extra info that is hashed
     */
    void* extra_hashed;
    int ext_h_alloc;
    int ext_h_size;
    /*
     Down pointers, indexes, edge values.
     */
    node_handle* down;
    int* index;
    void* edge;
    int alloc;
    int ealloc;
    int size;
    int nnzs;
    int level;
    unsigned h;
    char edge_bytes; // number of bytes for an edge value.
    bool is_full;
#ifdef DEVELOPMENT_CODE
    bool has_hash;
#endif
};



// ******************************************************************
// *                                                                *
// *                     unpacked_matrix  class                     *
// *                                                                *
// ******************************************************************

/** Class for unpacked matrices: unprimed, primed "2 levels" of nodes.
    Implemented in node_wrappers.cc.

    TBD : inline what we should
*/
class MEDDLY::unpacked_matrix {

  public:
    /**
      Different internal storage types.
      Affects the allowed read/write access methods.
    */
    enum storage_type {
      FULL_FULL     = 0,  // Full storage; acts like a 2-d array
      FULL_SPARSE   = 1,  // Compressed row storage; each row is sparse
      SPARSE_SPARSE = 2   // Coordinate list storage; list of (i, j, down)
    };

  public:
    unpacked_matrix();
    ~unpacked_matrix();

    /**
        An instance must be initialized, using init(), before use.
        Ties the instance to a particular level of a forest.
        Can be re-initialized with a different level or forest.
          @param  k   Level.
          @param  p   Forest.
    */
    void init(int k, const expert_forest* p);
    
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

      Basic matrix information
        
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /// How the matrix can be accessed.
    storage_type howStored() const;

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

      Methods for filling the unpacked matrix from nodes,
      primarily used by forests.
        
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /// Clear node and use FULL_FULL storage
    void initFullFull(int rows, int cols);

    /// Clear node and use FULL_SPARSE storage
    void initFullSparse(int rows, int maxnzs);

    /// Clear node and use SPARSE_SPARSE storage
    void initSparseSparse(int maxnzs);

    /** Add a non-zero element to the matrix.
        Must be called in lexicographical order of rows, columns.
        I.e., enumerate the rows in increasing order, and for each row,
        enumerate the columns in increasing order.
    */
    void appendElement(int i, int j, node_handle d);

    // tbd - edge version of appendElement.

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

      FULL_FULL storage access:  methods for reading and modifying
      matrix entries.
        
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    node_handle down(int i, int j) const;

    void set_down(int i, int j, node_handle d);


    // tbd - edges?


    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

      FULL_SPARSE and SPARSE_SPARSE storage access:  
      methods for reading matrix entries.
        
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /** 
      For FULL_SPARSE: Get nonzero index for first element in row i.
      Equivalently, get one plus the nonzero index for last element
      in row i-1.
        @param  i   Row index, between 0 and number of rows (inclusive).
        @return     A nonzero index, can be used in methods col_index()
                    and down().
    */
    int row_start(int i) const; 

    /// For SPARSE_SPARSE: Get row for nonzero number z.
    int row_index(int z) const;

    /// Get column for nonzero number z.
    int col_index(int z) const;

    /// Get downward pointer for nonzero number z.
    node_handle down(int z) const;

    // tbd - edges?

  private:
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

      Implementation details.  Nothing to see here.
        
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


};  // end of unpacked_matrix class


// ******************************************************************
// *                                                                *
// *                     initializer_list class                     *
// *                                                                *
// ******************************************************************

/** Mechanism for initializing and/or cleaning up library structures.
    Any user additions to the library should utilize this class.
    Derive a class from this one, provide the \a setup and \a cleanup
    methods.
    Implementation in meddly.cc
*/
class MEDDLY::initializer_list {
  public:
    /**
        Constructor.
        Takes the initializer(s) to run before this one.
        Cleanup runs in the reverse order.
    */
    initializer_list(initializer_list* previous);
    virtual ~initializer_list();

    /**
        Run all setup methods for the list of initializers,
        "previous first".
    */
    void setupAll();

    /**
        Run all cleanup methods for the list of initializers,
        "previous last".
    */
    void cleanupAll();

  protected:
    virtual void setup() = 0;
    virtual void cleanup() = 0;

  private:
    initializer_list* previous;
};

// ******************************************************************
// *                                                                *
// *                    node_storage_style class                    *
// *                                                                *
// ******************************************************************

/** Abstract base class for node storage factories.

    The base class is implemented in node_wrappers.cc;
    various backends are implemented in directory storage/.

*/
class MEDDLY::node_storage_style {
  public:
    node_storage_style();
    virtual ~node_storage_style();


    /** Build a new node storage mechanism, bound to the given forest.

          @param  f   Forest to bind to

          @return     A pointer to a node storage class,
                      initialized for forest f.
    */
    virtual node_storage* createForForest(expert_forest* f) const = 0;

};

// ******************************************************************
// *                                                                *
// *                       node_storage class                       *
// *                                                                *
// ******************************************************************

/** Abstract base class for node storage.

    The base class is implemented in node_wrappers.cc;
    various backends are implemented in directory storage/,
    or you may implement your own scheme :^)

    Nodes are represented by an address, which for a valid
    node, will be greater than 0.  The first valid address
    must be 1.

    Whatever scheme is used to store nodes internally,
    it must be possible to set pointers \a count and \a next
    such that count[addr] and next[addr] give the incoming
    count and next pointer for the given node addr.
    Derived classes are responsible for setting up
    and maintaining these pointers.
*/
class MEDDLY::node_storage {
  public:
    node_storage(expert_forest* f);
    virtual ~node_storage();

    /** Go through and collect any garbage.

          @param  shrink    If true, we will shrink data structures
                            as we free space; otherwise, we won't.
    */
    virtual void collectGarbage(bool shrink) = 0;

    /** Show various stats.
          @param  s       Output stream to write to
          @param  pad     Padding string, written at the start of
                          each output line.
          @param  flags   Controls what is displayed.
    */
    virtual void reportStats(output &s, const char* pad, unsigned flags) const = 0;

    /** Write a node in human-readable format.

        Ideally, the output format for each node is the same
        regardless of how it is stored.

        @param  s       Output stream.
        @param  addr    Address of the node we care about.
        @param  details Should we show "details" or not.
    */
    // virtual void showNode(output &s, node_address addr, bool details) const = 0;

    /** Write a node in machine-readable format.

        Ideally, the output format for each node is the same
        regardless of how it is stored.

        @param  s       Output stream.
        @param  addr    Address of the node we care about.
        @param  map     Translation to use on node handles.
                        Allows us to renumber nodes as we write them.
    */
    // virtual void writeNode(output &s, node_address addr, const node_handle* map) const;

    /** Dump the internal storage details.
        Primarily used for debugging.

          @param  s       Output stream to use
          @param  flags   What to show.
                            0x01  Show active memory
                            0x02  Show memory "holes"
    */
    void dumpInternal(output &s, unsigned flags) const;

    /** Allocate space for, and store, a node.
        I.e., create a new node that is a copy of the given one.
        The node might be "compressed" in various ways to reduce
        storage requirements.  (Indeed, that is the whole point
        of the node_storage class.)
            @param  p     Node handle number, in case it is used
            @param  nb    Node data is copied from here.
            @param  opt   Ways we can store the node.
            @return       The "address" of the new node.
    */
    virtual node_address makeNode(node_handle p, const unpacked_node &nb,
                                  node_storage_flags opt) = 0;

    /** Destroy a node.
        Unlink the downward pointers, and recycle the memory
        used by the node.
            @param  addr    Address of the node.
    */
    virtual void unlinkDownAndRecycle(node_address addr) = 0;


    // various ways to read a node

    /** Check for duplicates.
          @param  addr    Node address in this structure
          @param  nr      Node to compare against

          @return true    iff the nodes are duplicates
    */
    virtual bool
        areDuplicates(node_address addr, const unpacked_node &nr) const = 0;

    /**
        Copy the node at the specified address, into an unpacked node.
        Useful for reading an entire node.
          @param  un      Result will be stored here.  Will be resized if needed.
          @param  addr    Node address in this structure.
    */
    virtual void fillUnpacked(unpacked_node &un, node_address addr, unpacked_node::storage_style st2) const = 0;

    /** Compute the hash value for a node.
        Should give the same answer as filling a unpacked_node
        and computing the hash on the unpacked_node.

          @param  levl  Level of the node of interest
          @param  addr  Address of the node of interest
    */
    virtual unsigned hashNode(int level, node_address addr) const = 0;

    /** Determine if this is a singleton node.
        Used for identity reductions.
          @param  addr    Address of the node we care about
          @param  down    Output:
                          The singleton downward pointer, or undefined.

          @return   If the node has only one non-zero downward pointer,
                    then return the index for that pointer.
                    Otherwise, return a negative value.
    */
    virtual int
        getSingletonIndex(node_address addr, node_handle &down) const = 0;


    /** Get the specified downward pointer for a node.
        Fast if we just want one.
          @param  addr    Address of the node we care about
          @param  index   Index of downward pointer
          @return         Desired pointer
          @throw          INVALID_VARIABLE, if index is negative.
    */
    virtual node_handle getDownPtr(node_address addr, int index) const = 0;

    /** Get the specified outgoing edge for a node.
        Fast if we just want one.

          @param  addr    Address of the node we care about
          @param  ind     Index of the pointer we want.
          @param  ev      Output: edge value at that index.
          @param  dn      Output: downward pointer at that index.
    */
    virtual void getDownPtr(node_address addr, int ind, int& ev,
                            node_handle& dn) const = 0;

    /** Get the specified outgoing edge for a node.
        Fast if we just want one.

          @param  addr    Address of the node we care about
          @param  ind     Index of the pointer we want.
          @param  ev      Output: edge value at that index.
          @param  dn      Output: downward pointer at that index.
    */
    virtual void getDownPtr(node_address addr, int ind, float& ev,
                            node_handle& dn) const = 0;


    /** Read the unhashed header portion of a node.

          @param  addr    Address of the node we care about
    */
    virtual const void* getUnhashedHeaderOf(node_address addr) const = 0;

    /** Read the hashed header portion of a node.

          @param  addr    Address of the node we care about
    */
    virtual const void* getHashedHeaderOf(node_address addr) const = 0;

    // --------------------------------------------------
    // incoming count data
    // --------------------------------------------------

#ifdef INLINED_COUNT
    /// Get the number of incoming pointers to a node.
    int getCountOf(node_address addr) const;
    /// Set the number of incoming pointers to a node.
    void setCountOf(node_address addr, node_handle c);
    /// Increment (and return) the number of incoming pointers to a node.
    int incCountOf(node_address addr);
    /// Decrement (and return) the number of incoming pointers to a node.
    int decCountOf(node_address addr);
#else
    /// Get the number of incoming pointers to a node.
    virtual node_handle getCountOf(node_address addr) const;
    /// Set the number of incoming pointers to a node.
    virtual void setCountOf(node_address addr, node_handle c);
    /// Increment (and return) the number of incoming pointers to a node.
    virtual node_handle incCountOf(node_address addr);
    /// Decrement (and return) the number of incoming pointers to a node.
    virtual node_handle decCountOf(node_address addr);
#endif

    // --------------------------------------------------
    // next pointer data
    // --------------------------------------------------

#ifdef INLINED_NEXT
    node_handle getNextOf(node_address addr) const;
    void setNextOf(node_address addr, node_handle n);
#else
    virtual node_handle getNextOf(node_address addr) const;
    virtual void setNextOf(node_address addr, node_handle n);
#endif

  protected:
    /// Dump information not related to individual nodes.
    virtual void dumpInternalInfo(output &s) const = 0;

    /** Dump the node/hole information at the given address.
          @param  s       Output stream to use
          @param  addr    Address
          @param  flags   What chunks should be displayed

          @return   Next interesting address.
    */
    virtual node_address dumpInternalNode(output &s, node_address addr,
                                          unsigned flags) const = 0;

    /// Dump final info (after node info)
    virtual void dumpInternalTail(output &s) const = 0;

    // Hooks from other classes, so we don't need to make
    // all the derived classes "friends".

    void moveNodeOffset(node_handle node, node_address old_addr,
                        node_address new_addr);

    //
    // Methods for derived classes to deal with
    // members owned by the base class
    //

    const expert_forest* getParent() const;
    expert_forest* getParent();

    void incMemUsed(long delta);
    void decMemUsed(long delta);
    void incMemAlloc(long delta);
    void decMemAlloc(long delta);
    void incCompactions();
    void updateCountArray(node_handle* cptr);
    void updateNextArray(node_handle* nptr);

    //
    // Hooks for hole managers
    //

    // Change the data array
    virtual void updateData(node_handle* data) = 0;

    // How small can a node be?
    virtual int smallestNode() const = 0;

    friend class holeman;

  private:
    /// Parent forest.
    expert_forest* parent;

    /// Memory stats
    forest::statset* stats;

    /// Count array, so that counts[addr] gives the count for node at addr.
    int* counts;

    /// Next array, so that nexts[addr] gives the next value for node at addr.
    node_handle* nexts;
};


// ******************************************************************
// *                                                                *
// *                                                                *
// *                      expert_forest  class                      *
// *                                                                *
// *                                                                *
// ******************************************************************

class MEDDLY::expert_forest: public forest
{
    // flags for reporting; DO NOT rely on specific values
  public:
    /// Should memory be reported in a human readable format
    static const unsigned HUMAN_READABLE_MEMORY;
    /// Basic forest stats
    static const unsigned BASIC_STATS;
    static const unsigned EXTRA_STATS;
    /// Specific forest stats, dependent on forest type
    static const unsigned FOREST_STATS;
    /// Stats specific to the node storage mechanism.
    static const unsigned STORAGE_STATS;
    /// Detailed stats for the node storage mechanism.
    static const unsigned STORAGE_DETAILED;
    /// Stats specific to the unique table.
    static const unsigned UNIQUE_TABLE_STATS;
    /// Stats specific to the unique table.
    static const unsigned UNIQUE_TABLE_DETAILED;
    /// Stats specific to the hole manager.
    static const unsigned HOLE_MANAGER_STATS;
    /// Stats specific to the hole manager.
    static const unsigned HOLE_MANAGER_DETAILED;

    // ************************************************************
    // *                                                          *
    // *               Constants for  showing nodes               *
    // *                                                          *
    // ************************************************************

    /// Display deleted nodes
    static const unsigned int SHOW_DELETED;
    /// Display zombie nodes
    static const unsigned int SHOW_ZOMBIE;
    /// Display node details
    static const unsigned int SHOW_DETAILS;
    /// Display the node index
    static const unsigned int SHOW_INDEX;
    /// Display terminal nodes
    static const unsigned int SHOW_TERMINALS;

    // ************************************************************
    // *                                                          *
    // *    Preferred way to encode and decode terminal values    *
    // *    (classes so we can use them in template functions)    *
    // *                                                          *
    // ************************************************************

    /** Encoding for booleans into (terminal) node handles */
    class bool_Tencoder {
        // -1 true
        //  0 false
      public:
        static node_handle value2handle(bool v);
        static bool handle2value(node_handle h);
        static void show(output &s, node_handle h);
        static void write(output &s, node_handle h);
        static node_handle read(input &s);
    };
    /** Encoding for integers into (terminal) node handles */
    class int_Tencoder {
      public:
        static node_handle value2handle(int v);
        static int handle2value(node_handle h);
        static void show(output &s, node_handle h);
        static void write(output &s, node_handle h);
        static node_handle read(input &s);
    };
    /** Encoding for floats into (terminal) node handles */
    class float_Tencoder {
        union intfloat {
            float real;
            int integer;
        };
      public:
        static node_handle value2handle(float v);
        static float handle2value(node_handle h);
        static void show(output &s, node_handle h);
        static void write(output &s, node_handle h);
        static node_handle read(input &s);
    };
    // preferred way to encode and decode edge values
    // (classes so we can use them in template functions)
    /** Encoding for ints into (edge value) node handles */
    class int_EVencoder {
      public:
        static size_t edgeBytes();
        static void writeValue(void* ptr, int val);
        static void readValue(const void* ptr, int &val);
        static void show(output &s, const void* ptr);
        static void write(output &s, const void* ptr);
        static void read(input &s, void* ptr);
    };
    /** Encoding for floats into (edge value) node handles */
    class float_EVencoder {
      public:
        static size_t edgeBytes();
        static void writeValue(void* ptr, float val);
        static void readValue(const void* ptr, float &val);
        static void show(output &s, const void* ptr);
        static void write(output &s, const void* ptr);
        static void read(input &s, void* ptr);
    };


    friend class reordering_base;

    /** Constructor.
      @param  dslot   slot used to store the forest, in the domain
      @param  d       domain to which this forest belongs to.
      @param  rel     does this forest represent a relation.
      @param  t       the range of the functions represented in this forest.
      @param  ev      edge annotation.
      @param  p       Polcies for reduction, storage, deletion.
    */
    expert_forest(int dslot, domain *d, bool rel, range_type t,
                  edge_labeling ev, const policies &p,int* level_reduction_rule);

  // ------------------------------------------------------------
  // inlined helpers.

    /**
        Convenience function.
        Based on the forest type, convert the desired value
        into a terminal node handle.
          @param  v   Value to encode
          @return     Handle for terminal node
    */
    template <typename T>
    node_handle handleForValue(T v) const;

    /**
        Convenience function.
        Based on the forest type, convert the terminal node handle
        into its encoded value.
          @param  n   Node handle
          @param  v   Output: encoded value
    */
    template <typename T>
    void getValueFromHandle(node_handle n, T& v) const;

    /**
        Convenience function.
        Based on the forest type, convert the terminal node handle
        into its encoded boolean value.
          @param  n   Node handle
    */
    bool getBooleanFromHandle(node_handle n) const;

    /**
        Convenience function.
        Based on the forest type, convert the terminal node handle
        into its encoded integer value.
          @param  n   Node handle
    */
    int getIntegerFromHandle(node_handle n) const;

    /**
        Convenience function.
        Based on the forest type, convert the terminal node handle
        into its encoded real (float) value.
          @param  n   Node handle
    */
    float getRealFromHandle(node_handle n) const;

    statset& changeStats();
    /// Number of bytes for an edge value.
    char edgeBytes() const;
    /// Are edge values included when computing the hash.
    bool areEdgeValuesHashed() const;
    /// Extra bytes per node, not hashed.
    char unhashedHeaderBytes() const;
    /// Extra bytes per node, hashed.
    char hashedHeaderBytes() const;

    const expert_domain* getExpertDomain() const;
    expert_domain* useExpertDomain();

  // --------------------------------------------------
  // Node address information
  // --------------------------------------------------
  protected:
    node_address getNodeAddress(node_handle p) const;
    void setNodeAddress(node_handle p, node_address a);

  // --------------------------------------------------
  // Node level information
  // --------------------------------------------------
  public:
    /**
        Negative values are used for primed levels or variables.
    */
    int getVarByLevel(int level) const {
      return level > 0 ? var_order->getVarByLevel(level) : -var_order->getVarByLevel(-level);
    }
    int getLevelByVar(int var) const {
      return var > 0 ? var_order->getLevelByVar(var) : -var_order->getLevelByVar(-var);
    }

    int getNodeLevel(node_handle p) const;
    bool isPrimedNode(node_handle p) const;
    bool isUnprimedNode(node_handle p) const;
    int getNumVariables() const;
    // returns 0 or -K 
    int getMinLevelIndex() const;
    bool isValidLevel(int k) const;

    /// The maximum size (number of indices) a node at this level can have
    int getLevelSize(int lh) const;
    // The maximum size (number of indices) a variable can have.
    int getVariableSize(int var) const;

  protected:
    void setNodeLevel(node_handle p, int level);

  // --------------------------------------------------
  // Managing incoming edge counts
  // --------------------------------------------------
  public:
    /// Returns true if we are tracking incoming counts
    bool trackingInCounts() const;

    /// Returns the in-count for a node.
    // long readInCount(node_handle p) const;
    int getNodeInCount(node_handle p) const;

    /** Increase the link count to this node. Call this when another node is
        made to point to this node.
          @return p, for convenience.
    */
    node_handle linkNode(node_handle p);

    /** Decrease the link count to this node. If link count reduces to 0, this
        node may get marked for deletion. Call this when another node releases
        its connection to this node.
    */
    void unlinkNode(node_handle p);

  // --------------------------------------------------
  // Managing cache counts
  // --------------------------------------------------
  public:
    /// Returns true if we are tracking incoming counts
    bool trackingCacheCounts() const;

    /// Returns the cache count for a node.
    int getNodeCacheCount(node_handle p) const;

    /** Increase the cache count for this node. Call this whenever this node
        is added to a cache.
          @param  p     Node we care about.
          @return p, for convenience.
    */
    node_handle cacheNode(node_handle p);

    /** Increase the cache count for this node. Call this whenever this node
        is added to a cache.
          @param  p     Node we care about.
    */
    void uncacheNode(node_handle p);

  // --------------------------------------------------
  // Marking and unmarking nodes
  // --------------------------------------------------
  public:
    /// Set all nodes as marked
    void markAllNodes();

    /// Set all nodes as unmarked
    void unmarkAllNodes();

    /// Mark a particular node
    void markNode(node_handle p);

    /// Unmark a particular node
    void unmarkNode(node_handle p);

    /// Determine if a node is marked
    bool isNodeMarked(node_handle p) const;


  // --------------------------------------------------
  // Node status
  // --------------------------------------------------
  public:
    bool isActiveNode(node_handle p) const;
    bool isZombieNode(node_handle p) const;
    bool isDeletedNode(node_handle p) const;
    static bool isTerminalNode(node_handle p);
    /// Sanity check: is this a valid nonterminal node index.
    bool isValidNonterminalIndex(node_handle p) const;
    /// Sanity check: is this a valid node index.
    bool isValidNodeIndex(node_handle p) const;
    node_handle getLastNode() const;


  public:

    /// Get the cardinality of an Index Set.
    int getIndexSetCardinality(node_handle node) const;

    // --------------------------------------------------
    // Used by the unique table
    // --------------------------------------------------
    node_handle getNext(node_handle p) const;
    void setNext(node_handle p, node_handle n);
    unsigned hash(node_handle p) const;


    /// A node can be discarded once it goes stale. Whether a node is
    /// considered stale depends on the forest's deletion policy.
    /// Optimistic deletion: A node is said to be stale only when both the
    ///   in-count and cache-count are zero.
    /// Pessimistic deletion: A node is said to be stale when the in-count
    ///  is zero regardless of the cache-count.
    bool isStale(node_handle node) const;

  // ------------------------------------------------------------
  // non-virtual, handy methods for debugging or logging.

    /**
        Display all nodes in the forest.
          @param  s       File stream to write to
          @param  flags   Switches to control output;
                          see constants "SHOW_DETAILED", etc.
    */
    void dump(output &s, unsigned int flags) const;
    void dumpInternal(output &s) const;
    void dumpUniqueTable(output &s) const;
    void validateIncounts(bool exact);
    void countNodesByLevel(long* active) const;


  // ------------------------------------------------------------
  // non-virtual, handy methods.

    /** Build a list of nodes in the subgraph below the given node.
        This for example is used to determine which nodes must
        be printed to display a subgraph.
        Terminal nodes are NOT included.

          @param  roots   Array of root nodes in the forest.
                          Each root node will be included in the list,
                          except for terminal nodes.

          @param  N       Dimension of \a roots array.

          @param  sort    If true, the list will be in increasing order.
                          Otherwise, the list will be in some convenient order
                          (currently, it is the order that nodes
                          are discovered).

          @return   A malloc'd array of non-terminal nodes, terminated by 0.
                    Or, a null pointer, if the list is empty.
    */
    node_handle*
    markNodesInSubgraph(const node_handle* roots, int N, bool sort) const;

    /** Count and return the number of non-terminal nodes
        in the subgraph below the given node.
    */
    long getNodeCount(node_handle node) const;

    /** Count and return the number of edges
        in the subgraph below the given node.
    */
    long getEdgeCount(node_handle node, bool countZeroes) const;

    /** Display the contents of a single node.
          @param  s       File stream to write to.
          @param  node    Node to display.
          @param  flags   Switches to control output;
                          see constants "SHOW_DETAILED", etc.

          @return true, iff we displayed anything
    */
    bool showNode(output &s, node_handle node, unsigned int flags = 0) const;

    /// Show all the nodes in the subgraph below the given nodes.
    void showNodeGraph(output &s, const node_handle* node, int n) const;


    /** Show various stats for this forest.
          @param  s       Output stream to write to
          @param  pad     Padding string, written at the start of
                          each output line.
          @param  flags   Which stats to display, as "flags";
                          use bitwise or to combine values.
                          For example, BASIC_STATS | FOREST_STATS.
    */
    void reportStats(output &s, const char* pad, unsigned flags) const;


    /// Compute a hash for a node.
    unsigned hashNode(node_handle p) const;

    /** Check and find the index of a single downward pointer.

          @param  node    Node we care about
          @param  down    Output:
                          The singleton downward pointer, or undefined.

          @return   If the node has only one non-zero downward pointer,
                    then return the index for that pointer.
                    Otherwise, return a negative value.
    */
    int getSingletonIndex(node_handle p, node_handle &down) const;

    /** Check and get a single downward pointer.

          @param  node    Node we care about
          @param  index   Index we're trying to match

          @return   If the only non-zero downward pointer for
                    this node happens at \a index, then return the pointer.
                    Otherwise, return 0.
    */
    node_handle getSingletonDown(node_handle node, int index) const;

    /** For a given node, get a specified downward pointer.

        This is designed to be used for one or two indexes only.
        For reading all or several downward pointers, a
        unpacked_node should be used instead.

          @param  p       Node to look at
          @param  index   Index of the pointer we want.

          @return         The downward pointer at that index.
    */
    node_handle getDownPtr(node_handle p, int index) const;

    /** For a given node, get a specified downward pointer.

        This is designed to be used for one or two indexes only.
        For reading all or several downward pointers, a
        unpacked_node should be used instead.

          @param  p       Node to look at
          @param  index   Index of the pointer we want.

          @param  ev      Output: edge value at that index.
          @param  dn      Output: downward pointer at that index.
    */
    void getDownPtr(node_handle p, int index, int& ev, node_handle& dn) const;

    /** For a given node, get a specified downward pointer.

        This is designed to be used for one or two indexes only.
        For reading all or several downward pointers, a
        unpacked_node should be used instead.

          @param  p       Node to look at
          @param  index   Index of the pointer we want.

          @param  ev      Output: edge value at that index.
          @param  dn      Output: downward pointer at that index.
    */
    void getDownPtr(node_handle p, int index, float& ev, node_handle& dn) const;

    node_handle getTransparentNode() const;

  // ------------------------------------------------------------
  // Copy a node into an unpacked node

  void fillUnpacked(unpacked_node &un, node_handle node, unpacked_node::storage_style st2) const;

  /**   Return a forest node equal to the one given.
        The node is constructed as necessary.
        This version should be used only for
        multi terminal forests.
        The unpacked node is recycled.
          @param  in    Incoming pointer index;
                        used for identity reductions.
          @param  un    Temporary node; will be recycled.

          @return       A node handle equivalent
                        to \a un, taking into account
                        the forest reduction rules
                        and if a duplicate node exists.
    
  */
  node_handle createReducedNode(int in, unpacked_node *un);

  /** Return a forest node equal to the one given.
      The node is constructed as necessary.
      This version should be used only for
      edge valuded forests.
      The node builder nb is recycled.
        @param  in    Incoming pointer index;
                      used for identity reductions.
        @param  nb    Constructed node.
        @param  ev    Output: edge value
        @param  node  Output: node handle.
                      On exit, the edge value and the node
                      handle together are equivalent to nb;
                      taking into account the forest reduction rules
                      and if a duplicate node exists.
  */
  template <class T>
  void createReducedNode(int in, unpacked_node* nb, T& ev, node_handle& node);


    /** Swap the content of nodes.
        Do not update their parents and inCount.
    */
    void swapNodes(node_handle p, node_handle q);

    /*
     * Modify a node in place.
     * Does not check if the modified node is duplicate or redundant.
     * The level of the node may change.
     * Keep the reference number and the cache count of the node.
     */
    node_handle modifyReducedNodeInPlace(unpacked_node* un, node_handle p);

  // ------------------------------------------------------------
  // virtual in the base class, but implemented here.
  // See meddly.h for descriptions of these methods.

    virtual void writeEdges(output &s, const dd_edge* E, int n) const;
    virtual void readEdges(input &s, dd_edge* E, int n);
    virtual void garbageCollect();
    virtual void compactMemory();
    virtual void showInfo(output &strm, int verbosity);

  // ------------------------------------------------------------
  // abstract virtual, must be overridden.
  //

    /** Are the given edge values "duplicates".
        I.e., when determining if two nodes are duplicates,
        do the given edge values qualify as duplicate values.
          @param  eva     Pointer to the first edge value
          @param  evb     Pointer to the second edge value

          @return     true, iff the edge values are "equal".
          @throws     A TYPE_MISMATCH error if the forest
                      does not store edge values.
    */
    virtual bool areEdgeValuesEqual(const void* eva, const void* evb) const;


    /** Discover duplicate nodes.
        Right now, used for sanity checks only.
          @param  node    Handle to a node.
          @param  nr      Some other node.

          @return   true, iff the nodes are duplicates.
    */
    bool areDuplicates(node_handle node, const unpacked_node &nr) const;

    /** Is this a redundant node that can be eliminated?
        Must be implemented in derived forests
        to deal with the default edge value.
          @param  nb    Node we're trying to build.

          @return   True, if nr is a redundant node
                          AND it should be eliminated.
    */
    virtual bool isRedundant(const unpacked_node &nb) const = 0;

    /** Is the specified edge an identity edge, that can be eliminated?
        Must be implemented in derived forests
        to deal with the default edge value.

          @param  nr    Node we're trying to build.
                        We know there is a single non-zero downward pointer.

          @param  i     Candidate edge (or edge index for sparse nodes).

          @return True, if nr[i] is an identity edge, and the
                        identity node should be eliminated.
    */
    virtual bool isIdentityEdge(const unpacked_node &nb, int i) const = 0;


    /**
        Build an iterator.
        Used by class enumerator.
    */
    virtual enumerator::iterator* makeFullIter() const = 0;

    /**
        Build an iterator with a fixed row.
        Default behavior - throw an "INVALID_FOREST" error.
    */
    virtual enumerator::iterator* makeFixedRowIter() const;

    /**
        Build an iterator with a fixed column.
        Default behavior - throw an "INVALID_FOREST" error.
    */
    virtual enumerator::iterator* makeFixedColumnIter() const;

    /*
     * Reorganize the variables in a certain order.
     */
    void reorderVariables(const int* level2var);

    void getVariableOrder(int* level2var) const;

    std::shared_ptr<const variable_order> variableOrder() const;

    /*
     * Swap the variables at level and level+1.
     * This method should only be called by expert_domain.
     */
    virtual void swapAdjacentVariables(int level) = 0;

    /*
     * Move the variable at level high down to level low.
     * The variables from level low to level high-1 will be moved one level up.
     */
    virtual void moveDownVariable(int high, int low) = 0;

    /*
     * Move the variable at level low up to level high.
     * The variables from level low+1 to level high will be moved one level down.
     */
    virtual void moveUpVariable(int low, int high) = 0;

    virtual void dynamicReorderVariables(int /*top*/, int /*bottom*/) {
    	throw error(error::NOT_IMPLEMENTED);
    }

    /** Show a terminal node.
          @param  s       Stream to write to.
          @param  tnode   Handle to a terminal node.
    */
    virtual void showTerminal(output &s, node_handle tnode) const;

    /** Write a terminal node in machine-readable format.
          @param  s       Stream to write to.
          @param  tnode   Handle to a terminal node.
    */
    virtual void writeTerminal(output &s, node_handle tnode) const;

    /** Read a terminal node in machine-readable format.
          @param  s       Stream to read from.
          @return         Handle to a terminal node.
          @throws         An invalid file exception if the stream does not
                          contain a valid terminal node.
    */
    virtual node_handle readTerminal(input &s);

    /** Show an edge value.
          @param  s       Stream to write to.
          @param  edge    Pointer to edge value chunk
    */
    virtual void showEdgeValue(output &s, const void* edge) const;

    /** Write an edge value in machine-readable format.
          @param  s       Stream to write to.
          @param  edge    Pointer to edge value chunk
    */
    virtual void writeEdgeValue(output &s, const void* edge) const;

    /** Read an edge value in machine-readable format.
          @param  s       Stream to read from.
          @param  edge    Pointer to edge value chunk
    */
    virtual void readEdgeValue(input &s, void* edge);

    /** Show the hashed header values.
          @param  s       Stream to write to.
          @param  hh      Pointer to hashed header data.
    */
    virtual void showHashedHeader(output &s, const void* hh) const;

    /** Write the hashed header in machine-readable format.
          @param  s       Stream to write to.
          @param  hh      Pointer to hashed header data.
    */
    virtual void writeHashedHeader(output &s, const void* hh) const;

    /** Read the hashed header in machine-readable format.
          @param  s       Stream to write to.
          @param  nb      Node we're building.
    */
    virtual void readHashedHeader(input &s, unpacked_node &nb) const;

    /** Show the unhashed header values.
          @param  s       Stream to write to.
          @param  uh      Array of all unhashed header values.
    */
    virtual void showUnhashedHeader(output &s, const void* uh) const;

    /** Write the unhashed header in machine-readable format.
          @param  s       Stream to write to.
          @param  hh      Pointer to unhashed header data.
    */
    virtual void writeUnhashedHeader(output &s, const void* uh) const;

    /** Read the unhashed header in machine-readable format.
          @param  s       Stream to write to.
          @param  nb      Node we're building.
    */
    virtual void readUnhashedHeader(input &s, unpacked_node &nb) const;



  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // |                                                                |
  // |                       protected  methods                       |
  // |                                                                |
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  protected:

    /// Destructor.
    virtual ~expert_forest();

    /** Initialize data.
        Should be called in the child class constructors.
        Allows us to use class properties to initialize the data.
    */
    void initializeForest();


  // ------------------------------------------------------------
  // inlined setters for derived classes to use.

    void setEdgeSize(char ebytes, bool hashed);
    void setUnhashedSize(char ubytes);
    void setHashedSize(char hbytes);

  // ------------------------------------------------------------
  // virtual, with default implementation.
  // Should be overridden in appropriate derived classes.

    /// Character sequence used when writing forests to files.
    virtual const char* codeChars() const;


    /** Normalize a node.
        Used only for "edge valued" DDs with range type: integer.
        Different forest types will have different normalization rules,
        so the default behavior given here (throw an error) will need
        to be overridden by all edge-valued forests.

          @param  nb    Array of downward pointers and edge values;
                        may be modified.
          @param  ev    The incoming edge value, may be modified
                        as appropriate to normalize the node.
    */
    virtual void normalize(unpacked_node &nb, int& ev) const;

    /** Normalize a node.
        Used only for "edge valued" DDs with range type: real.
        Different forest types will have different normalization rules,
        so the default behavior given here (throw an error) will need
        to be overridden by all edge-valued forests.

          @param  nb    Array of downward pointers and edge values;
                        may be modified.
          @param  ev    The incoming edge value, may be modified
                        as appropriate to normalize the node.
    */
    virtual void normalize(unpacked_node &nb, float& ev) const;

    /** Show forest-specific stats.
          @param  s     Output stream to use
          @param  pad   String to display at the beginning of each line.
    */
    virtual void reportForestStats(output &s, const char* pad) const;



  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // |                                                                |
  // |                        private  methods                        |
  // |                                                                |
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  private:

  // ------------------------------------------------------------
  // inlined helpers for this class

    bool isTimeToGc() const;

    /// Increment and return the in-count for a node
    // long incInCount(node_handle p);
    /// Decrement and return the in-count for a node
    // long decInCount(node_handle p);
    /// Returns the (modifiable) cache-count for a node
    // int& cacheCount(node_handle p);
    /// Returns the cache-count for a node
    // int getCacheCount(node_handle p) const;

    /** Change the location of a node.
        Used by node_storage during compaction.
        Should not be called by anything else.
          @param  node        Node we're moving
          @param  old_addr    Current address of node, for sanity check
          @param  new_addr    Where we're moving the node to
    */
    void moveNodeOffset(node_handle node, node_address old_addr, node_address new_addr);
    friend class MEDDLY::node_storage;

    friend class MEDDLY::global_rebuilder;

  // ------------------------------------------------------------
  // helpers for this class

    void handleNewOrphanNode(node_handle node);
    void deleteNode(node_handle p);
    void zombifyNode(node_handle p);

    /// Determine a node handle that we can use.
    node_handle getFreeNodeHandle();

    /// Release a node handle back to the free pool.
    void recycleNodeHandle(node_handle p);


    /** Apply reduction rule to the temporary node and finalize it. 
        Once a node is reduced, its contents cannot be modified.
          @param  in    Incoming index, used only for identity reduction;
                        Or -1.
          @param  un    Unpacked node.
          @return       Handle to a node that encodes the same thing.
    */
    node_handle createReducedHelper(int in, const unpacked_node &nb);

    // Sanity check; used in development code.
    void validateDownPointers(const unpacked_node &nb) const;

    /// Increase the number of node handles.
    void expandHandleList();

    /// Decrease the number of node handles.
    void shrinkHandleList();

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // |                                                                |
  // |                              Data                              |
  // |                                                                |
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  protected:
    /// uniqueness table, still used by derived classes.
    unique_table* unique;

    /// Should a terminal node be considered a stale entry in the compute table.
    /// per-forest policy, derived classes may change as appropriate.
    bool terminalNodesAreStale;

    /// Class that stores nodes.
    node_storage* nodeMan;

    /// Transparent value
    node_handle transparent;

    std::shared_ptr<const variable_order> var_order;

  private:
      /** Header information for each node.
          The node handles (integers) need to keep some
          information about the node they point to,
          both for addressing and for bookkeeping purposes.
          This struct holds that information.
      */
      struct node_header {
          /** Offset to node's data in the corresponding node storage structure.
              If the node is active, this is the offset (>0) in the data array.
              If the node is deleted, this is -next deleted node
              (part of the unused address list).
          */
          node_address offset;

          /** Node level
              If the node is active, this indicates node level.
          */
          int level;

          /** Cache count
              The number of cache entries that refer to this node (excl. unique
              table). If this node is a zombie, cache_count is negative.
          */
          int cache_count;

          /// Is node marked.  This is pretty horrible, but is only temporary
          bool marked;

          // Handy functions, in case our internal storage changes.

          bool isActive() const;
          bool isZombie() const;

          bool isDeleted() const;
          void setDeleted();
          void setNotDeleted();

          int getNextDeleted() const;
          void setNextDeleted(int n);

          void makeZombie();
      };


  private:
    // Garbage collection in progress
    bool performing_gc;

    // memory for validating incounts
    node_handle* in_validate;
    int  in_val_size;
    // depth of delete/zombie stack; validate when 0
    int delete_depth;

    /// address info for nodes
    node_header *address;
    /// Size of address/next array.
    node_handle a_size;
    /// Last used address.
    node_handle a_last;
    /// Pointer to unused address lists, based on size
    node_handle a_unused[8];  // number of bytes per handle
    /// Lowest non-empty address list
    char a_lowest_index;
    /// Next time we shink the address list.
    node_handle a_next_shrink;


    /// Number of bytes for an edge
    char edge_bytes;
    /// Are edge values hashed?
    bool hash_edge_values;
    /// Number of bytes of unhashed header
    char unhashed_bytes;
    /// Number of bytes of hashed header
    char hashed_bytes;

    class nodecounter;
};
// end of expert_forest class.


// ******************************************************************
// *                                                                *
// *                expert_forest::nodecounter class                *
// *                                                                *
// ******************************************************************

class MEDDLY::expert_forest::nodecounter: public edge_visitor {
    expert_forest* parent;
    int* counts;
  public:
    nodecounter(expert_forest*p, int* c);
    virtual ~nodecounter();
    virtual void visit(dd_edge &e);
};


// ******************************************************************
// *                                                                *
// *                          opname class                          *
// *                                                                *
// ******************************************************************

/// Class for names of operations.
class MEDDLY::opname {
    const char* name;
    int index;
    static int next_index;

    friend void MEDDLY::initialize(initializer_list *);
    friend void MEDDLY::cleanup();
  public:
    opname(const char* n);
    virtual ~opname();

    int getIndex() const;
    const char* getName() const;
};

// ******************************************************************
// *                                                                *
// *                       unary_opname class                       *
// *                                                                *
// ******************************************************************

/// Unary operation names.
class MEDDLY::unary_opname : public opname {
  public:
    unary_opname(const char* n);
    virtual ~unary_opname();

    virtual unary_operation*
      buildOperation(expert_forest* arg, expert_forest* res) const;

    virtual unary_operation*
      buildOperation(expert_forest* arg, opnd_type res) const;
};

// ******************************************************************
// *                                                                *
// *                      binary_opname  class                      *
// *                                                                *
// ******************************************************************

/// Binary operation names.
class MEDDLY::binary_opname : public opname {
  public:
    binary_opname(const char* n);
    virtual ~binary_opname();

    virtual binary_operation* buildOperation(expert_forest* arg1,
      expert_forest* arg2, expert_forest* res) const = 0;
};


// ******************************************************************
// *                                                                *
// *                    specialized_opname class                    *
// *                                                                *
// ******************************************************************

/// Specialized operation names.
class MEDDLY::specialized_opname : public opname {
  public:
    /**
      Abstract base class for arguments to buildOperation().
      Derived operation names must provide derived classes for arguments.
    */
    class arguments {
      public:
        arguments();
        virtual ~arguments();

        /**
            Specify if arguments should be destroyed or not.
            If yes (the default), the operation will destroy
            the arguments once they are no longer needed.
        */
        void setAutoDestroy(bool destroy);
        bool autoDestroy() const;

      private:
        bool destroyWhenDone;
    };

    specialized_opname(const char* n);
    virtual ~specialized_opname();

    /** Note - unlike the more general binary and unary ops,
        a specialized operation might be crafted to the specific
        arguments (passed as an abstract class).
        Examples are:
          - operations that will be called several times with
            several of the arguments unchanged, and we want
            to do some preprocessing on the unchanged arguments.

          - operations with very bizarre and/or user-defined
            parameters (like on-the-fly saturation).

        @param  a   Arguments.  Will be destroyed when we are finished,
                    if autoDestroy() is set for the arguments.
    */
    virtual specialized_operation* buildOperation(arguments* a) const = 0;
};

// ******************************************************************
// *                                                                *
// *                     numerical_opname class                     *
// *                                                                *
// ******************************************************************

/// Numerical operation names.
class MEDDLY::numerical_opname : public specialized_opname {
  public:
    class numerical_args : public specialized_opname::arguments {
      public:
        const dd_edge &x_ind;
        const dd_edge &A;
        const dd_edge &y_ind;

        numerical_args(const dd_edge &xi, const dd_edge &a, const dd_edge &yi);
        virtual ~numerical_args();
    };

    numerical_opname(const char* n);
    virtual ~numerical_opname();
    virtual specialized_operation* buildOperation(arguments* a) const = 0;

    /// For convenience, and backward compatability :^)
    specialized_operation* buildOperation(const dd_edge &x_ind,
      const dd_edge &A, const dd_edge &y_ind) const;
};


// ******************************************************************
// *                                                                *
// *                     satpregen_opname class                     *
// *                                                                *
// ******************************************************************

/// Saturation, with already generated transition relations, operation names.
class MEDDLY::satpregen_opname : public specialized_opname {
  public:
    satpregen_opname(const char* n);
    virtual ~satpregen_opname();

    /// Arguments should have type "pregen_relation".
    virtual specialized_operation* buildOperation(arguments* a) const = 0;

    /** Class for a partitioned transition relation, already known
        The relation can be partitioned "by events" or "by levels".
        In the case of "by events", we can have more than one relation
        per level; otherwise, there is at most one relation per level.
    */
    class pregen_relation : public specialized_opname::arguments {
      public:
        /** Constructor, by events
              @param  inmdd       MDD forest containing initial states
              @param  mxd         MxD forest containing relations
              @param  outmdd      MDD forest containing result
              @param  num_events  Number of events; specifies the maximum
                                  number of calls to addToRelation().
        */
        pregen_relation(forest* inmdd, forest* mxd, forest* outmdd,
          int num_events);

        /** Constructor, by levels
              @param  inmdd       MDD forest containing initial states
              @param  mxd         MxD forest containing relations
              @param  outmdd      MDD forest containing result
        */
        pregen_relation(forest* inmdd, forest* mxd, forest* outmdd);

        virtual ~pregen_relation();
        void addToRelation(const dd_edge &r);

        // Options for controlling the amount of processing performed by
        // \a finalize(splittingOption).
        enum splittingOption {
          // None.
          None,
          // Transitions from level K that do not effect level K,
          // are moved to a lower level.
          SplitOnly,
          // SplitOnly + duplicated transitions between adjacent levels
          // are removed from the higher level.
          SplitSubtract,
          // SplitOnly + all duplicate transitions are removed.
          SplitSubtractAll,
          // Same as SplitSubtractAll, but using an algorithm that
          // first combines all transitions before splitting it up per level.
          MonolithicSplit
        };

        /** To be called after all events have been added to
            the transition relation.
            This method modifies the decision diagrams stored at different
            levels, to reduce duplicated transitions.
              @param  split       This parameter only applies to "by levels",
                                  and it controls the amount of processing
                                  that is performed.
                                  Please refer to splittingOption for details.
        */
        void finalize(splittingOption split = SplitSubtract);

        bool isFinalized() const;

        forest* getInForest() const;
        forest* getRelForest() const;
        forest* getOutForest() const;

        // the following methods assume the relation has been finalized.
        node_handle* arrayForLevel(int k) const;
        int lengthForLevel(int k) const;

      private:
        // helper for constructors
        void setForests(forest* inf, forest* mxd, forest* outf);
        // helper for finalize,
        // find intersection of diagonals of events[k],
        // subtracts the intersection of events[k] and adds it to events[k-1].
        void splitMxd(splittingOption split);
        // helper for finalize
        // adds all event[k]; sets all event[k] to 0;
        // sets events[level(sum)] = sum
        void unionLevels();

        forest* insetF;
        expert_forest* mxdF;
        forest* outsetF;
        int K;
        // array of sub-relations
        node_handle* events;
        // next pointers, unless we're finalized
        int* next;
        // size of events array
        int num_events;
        // last used element of events array
        int last_event;

        // If null, then we are "by levels".  Otherwise, we are "by events",
        // and before we're finalized, level_index[k] points to a linked-list
        // of sub-relations that affect level k.
        // after we're finalized, the events array is sorted, so
        // level_index[k] is the index of the first event affecting level k.
        // Dimension is number of variables + 1.
        int* level_index;
    };

};



// ******************************************************************
// *                                                                *
// *                      satotf_opname  class                      *
// *                                                                *
// ******************************************************************

/// Saturation, transition relations built on the fly, operation names.
class MEDDLY::satotf_opname : public specialized_opname {
  public:
    satotf_opname(const char* n);
    virtual ~satotf_opname();

    /// Arguments should have type "otf_relation", below
    virtual specialized_operation* buildOperation(arguments* a) const = 0;

    class otf_relation;

    // ============================================================

    /**
        User must derive a subclass from this.
        Part of an enabling or updating function.
        It knows what variables it depends on, and how to build itself
        (provided by the user).
    */
    class subevent {
      public:
        /// Constructor, specify variables that this function depends on,
        /// and if it is a firing or enabling event.
        subevent(forest* f, int* v, int nv, bool firing);
        virtual ~subevent();

        /// Get the forest to which this function belongs to.
        expert_forest* getForest();

        /// Get number of variables this function depends on.
        int getNumVars() const;

        /// Get array of variables this function depends on.
        const int* getVars() const;

        /// Get the DD encoding of this function
        const dd_edge& getRoot() const;

        /// Get the "top" variable for this function
        int getTop() const;

        /// Is this a firing subevent?
        bool isFiring() const;

        /// Is this an enabling subevent
        bool isEnabling() const;

        /**
          Rebuild the function to include the
          local state "index" for the variable "v".
          Updates root with the updated function.
          User MUST provide this method.
        */
        virtual void confirm(otf_relation &rel, int v, int index) = 0;

        /// If num_minterms > 0,
        ///   Add all minterms to the root
        ///   Delete all minterms.
        void buildRoot();

        /// Debugging info
        void showInfo(output& out) const;

        long mintermMemoryUsage() const;
        void clearMinterms();

      protected:
        bool addMinterm(const int* from, const int* to);

        int* vars;
        int num_vars;
        dd_edge root;
        int top;
        expert_forest* f;
        int** unpminterms;
        int** pminterms;
        int num_minterms;
        int size_minterms;
        bool is_firing;

    };  // end of class subevent

    // ============================================================

    /**
        An "event".
        Produces part of the transition relation, from its sub-functions.

        TBD - do we need to split the enabling and updating sub-functions,
        or will one giant list work fine?
    */
    class event {
        // TBD - put a list of events that have priority over this one

        // TBD - for priority - when is this event enabled?
      public:
        event(subevent** se, int nse);
        virtual ~event();

        /// Get the forest to which the subevents belong to
        inline expert_forest* getForest() { return f; } 

        /// Get number of subevents
        inline int getNumOfSubevents() const { return num_subevents; }

        /// Get array of subevents
        inline subevent** getSubevents() const { return subevents; }

        /// Get the "top" variable for this event
        inline int getTop() const { return top; }

        /// Get the number of variables that are effected by this event
        inline int getNumVars() const { return num_vars; }

        /// Get a (sorted) array of variables that are effected by this event
        inline const int* getVars() const { return vars; }

        inline const dd_edge& getRoot() const { return root; }

        inline bool isDisabled() const { return is_disabled; }

        inline bool needsRebuilding() const { return needs_rebuilding; }

        inline void markForRebuilding() { needs_rebuilding = true; }

        /**
            If this event has been marked for rebuilding:
              Build this event as a conjunction of its sub-events.

            @return               true, if the event needed rebuilding and
                                        the rebuilding modified the root.
                                  false, otherwise.
        */
        virtual bool rebuild();

        /// Enlarges the "from" variable to be the same size as the "to" variable
        void enlargeVariables();

        /// Debugging info
        void showInfo(output& out) const;

        long mintermMemoryUsage() const;

      protected:
        void buildEventMask();

      private:
        subevent** subevents;
        int num_subevents;
        int top;
        int num_vars;
        int* vars;
        dd_edge root;
        bool needs_rebuilding;
        expert_forest* f;

        bool is_disabled;
        int num_firing_vars;
        int* firing_vars;
        dd_edge event_mask;
        int* event_mask_from_minterm;
        int* event_mask_to_minterm;

    };  // end of class event

    // ============================================================

    /**
        Overall relation.
        This includes all events, and keeping track of which local
        variables are confirmed.

        TBD.
    */
    class otf_relation : public specialized_opname::arguments {
      public:
        /** Constructor.
              @param  inmdd       MDD forest containing initial states
              @param  mxd         MxD forest containing relations
              @param  outmdd      MDD forest containing result
              @param  E           List of events
              @param  nE          Number of events
        */
        otf_relation(forest* inmdd, forest* mxd, forest* outmdd,
          event** E, int ne);

        virtual ~otf_relation();

        /// Returns the MDD forest that stores the initial set of states
        expert_forest* getInForest() const;

        /// Returns the MXD forest that stores the events
        expert_forest* getRelForest() const;

        /// Returns the MDD forest that stores the resultant set of states
        expert_forest* getOutForest() const;

        /// Returns true if the local state is already confirmed.
        bool isConfirmed(int level, int index) const;

        /// Returns an array of local states for this level, such that
        /// result[i] == isConfirmed(level, i).
        const bool* getLocalStates(int level);

        /// Returns the number of confirmed states at this level
        int getNumConfirmed(int level) const;

        /// Confirms all local states enabled in the given MDD
        void confirm(const dd_edge& set);

        /** Confirm a variable's previously unconfirmed state.
            Any event that is dependent on this variable is marked
            as "stale" --- so that it is rebuilt before use.

            @param  level       variable's level
            @param  index       the state of the variable being confirmed.
            @return             false: if state was previously confirmed.
                                true: if state was previously unconfirmed.
         */
        bool confirm(int level, int index);

        /** Get the number of events at whose "top" is this level.

            @param  level       level for the events.
            @return             number of events whose "top" is this level.
         */
        int getNumOfEvents(int level) const;

        /** Gets an event from the set of events whose "top" is this level.

            @param  level       level for the events.
            @param  i           index of the event.
            @return             if 0 <= i < getNumOfEvents(level),
                                the ith event at this level;
                                otherwise, 0.
         */
        node_handle getEvent(int level, int i);

        /** Rebuild an event.

            @param  i           index of the event.
            @return             true, if event was updated.
          */
        bool rebuildEvent(int level, int i);

        /// For Debugging
        void showInfo(output &strm) const;

        long mintermMemoryUsage() const;

        void clearMinterms();

      protected:
        void enlargeConfirmedArrays(int level, int sz);

      private:
        expert_forest* insetF;
        expert_forest* mxdF;
        expert_forest* outsetF;
        int num_levels;

        // All events that begin at level i,
        // are listed in events_by_top_level[i].
        // An event will appear in only one list
        // (as there is only one top level per event).
        // num_events_by_top_level[i] gives the size of events_by_top_level[i]
        event*** events_by_top_level;
        int *num_events_by_top_level;

        // All events that depend on a level i,
        // are listed in events_by_level[i]
        // Therefore, an event that depends on n levels,
        // will appear in n lists
        // num_events_by_level[i] gives the size of events_by_level[i]
        event*** events_by_level;
        int *num_events_by_level;

        // All subevents that depend on a level i,
        // are listed in subevents_by_level[i]
        // Therefore, an subevent that depends on n levels,
        // will appear in n lists
        // num_subevents_by_level[i] gives the size of subevents_by_level[i]
        subevent*** subevents_by_level;
        int *num_subevents_by_level;

        // List of confirmed local states at each level
        bool** confirmed;
        int* size_confirmed;
        int* num_confirmed;

    };  // end of class otf_relation

};  // end of class satotf_opname

// ******************************************************************
// *                                                                *
// *                         ct_object class                        *
// *                                                                *
// ******************************************************************

/** Generic objects in compute tables.
    Used for things other than dd_edges and simple types.
    Defined in ops.cc
*/
class MEDDLY::ct_object {
  public:
    ct_object();
    virtual ~ct_object();
    virtual opnd_type getType() = 0;
};

// under construction:


// ******************************************************************
// *                                                                *
// *                      ct_initializer  class                     *
// *                                                                *
// ******************************************************************

/** Interface for initializing Meddly's compute table(s).
    Implemented in compute_table.cc.
    Note - this is a singleton class but this is not enforced.
  
    This is exposed here because it allows us to avoid a
    "chicken and egg" problem:  to initialize the library, we want to 
    set the compute table style, but we cannot guarantee that those
    pointers are set up before we initialize the library.
    So, settings for compute tables should be made as follows.

    (1) call defaultInitializerList(), to build an instance of this class,
        and save the result.  That will set up the default settings.

    (2) change settings using static members

    (3) initialize Meddly using the saved initializer list.

*/
class MEDDLY::ct_initializer : public initializer_list {
  public:
    enum staleRemovalOption {
      /// Whenever we see a stale entry, remove it.
      Aggressive,
      /// Only remove stales when we need to expand the table
      Moderate,
      /// Only remove stales during Garbage Collection.
      Lazy
    };

    enum builtinCTstyle {
      /// One huge hash table that uses chaining.
      MonolithicChainedHash,

      /// One huge hash table that does not use chaining.
      MonolithicUnchainedHash,

      /// A hash table (with chaining) for each operation.
      OperationChainedHash,

      /// A hash table (no chaining) for each operation.
      OperationUnchainedHash,

      /// A STL "map" for each operation.
      OperationMap
    };

    struct settings {
      staleRemovalOption staleRemoval;
      unsigned maxSize;
    };

  public:
    ct_initializer(initializer_list* previous);
    virtual ~ct_initializer();

  protected:
    virtual void setup();
    virtual void cleanup();

  // use these to change defaults, before library initialization
  public:
    static void setStaleRemoval(staleRemovalOption sro);
    static void setMaxSize(unsigned ms);
    static void setBuiltinStyle(builtinCTstyle cts);
    static void setUserStyle(const compute_table_style*);

    // for convenience
    static compute_table* createForOp(operation* op);

  private:
    static settings the_settings;
    static const compute_table_style* ct_factory;
    static compute_table_style* builtin_ct_factory;
};

// ******************************************************************
// *                                                                *
// *                    compute_table_style class                   *
// *                                                                *
// ******************************************************************

/** Interface for building compute tables.
*/
class MEDDLY::compute_table_style {
  public:
    compute_table_style();
    virtual ~compute_table_style();

    /** Build a new, monolithic table.
        Monolithic means that the table stores entries for several
        (ideally, all) operations.

        Default throws an error.
    */
    virtual compute_table* create(const ct_initializer::settings &s)
      const;


    /**
        Build a new table for a single operation.
        Default throws an error.
    */
    virtual compute_table* create(const ct_initializer::settings &s,
      operation* op) const;


    /**
        Does this style build monolithic CTs?
    */
    virtual bool usesMonolithic() const = 0;
};

// ******************************************************************
// *                                                                *
// *                      compute_table  class                      *
// *                                                                *
// ******************************************************************

/** Interface for compute tables.
    Anyone implementing an operation (see below) will
    probably want to use this.
*/
class MEDDLY::compute_table {
    public:
      /// The maximum size of the hash table.
      unsigned maxSize;
      /// Do we try to eliminate stales during a "find" operation
      bool checkStalesOnFind;
      /// Do we try to eliminate stales during a "resize" operation
      bool checkStalesOnResize;

      struct stats {
        unsigned numEntries;
        long hits;
        long pings;
        static const int searchHistogramSize = 256;
        long searchHistogram[searchHistogramSize];
        long numLargeSearches;
        int maxSearchLength;
      };

      //
      // Something to search for in the CT.
      // This is an interface now!
      //
      class search_key {
          operation* op;

        protected:
          search_key(operation* op);

        public:
          /// Used for linked-list of recycled search keys in an operation.
          search_key* next;

          operation* getOp() const;
          virtual ~search_key();

          // interface, for operations
          virtual void reset() = 0;
          virtual void writeNH(node_handle nh) = 0;
          virtual void write(int i) = 0;
          virtual void write(float f) = 0;
      };

      //
      // Result of a search
      //
      class search_result {
          bool is_valid;

        protected:
          search_result();
          virtual ~search_result();

        public:
          void setValid();
          void setInvalid();
          operator bool() const;

          virtual node_handle readNH() = 0;
          virtual void read(int &i) = 0;
          virtual void read(float &f) = 0;
          virtual void read(long &l) = 0;
          virtual void read(double &d) = 0;
          virtual void read(void* &ptr) = 0;
      };

      //
      // Building a new CT entry.
      // This is an interface now!
      //
      class entry_builder {
        protected:
          entry_builder();
          virtual ~entry_builder();

        public:
          virtual void writeResultNH(node_handle) = 0;
          virtual void writeResult(int) = 0;
          virtual void writeResult(float) = 0;
          virtual void writeResult(long) = 0;
          virtual void writeResult(double) = 0;
          virtual void writeResult(void*) = 0;
      };

      // convenience methods, for grabbing edge values
      static void readEV(const node_handle* p, int &ev);
      static void readEV(const node_handle* p, float &ev);

      /// Constructor
      compute_table(const ct_initializer::settings &s);

      /** Destructor.
          Does NOT properly discard all table entries;
          use \a removeAll() for this.
      */
      virtual ~compute_table();

      /// Is this a per-operation compute table?
      virtual bool isOperationTable() const = 0;

      /// Initialize a search key for a given operation.
      virtual search_key* initializeSearchKey(operation* op) = 0;

      /** Find an entry in the compute table based on the key provided.
          @param  key   Key to search for.
          @return       An appropriate search_result.
      */
      virtual search_result& find(search_key *key) = 0;

      /** Start a new compute table entry.
          The operation should "fill in" the values for the entry,
          then call \a addEntry().
      */
      virtual entry_builder& startNewEntry(search_key* k) = 0;

      /** Add the "current" new entry to the compute table.
          The entry may be specified by filling in the values
          for the struct returned by \a startNewEntry().
      */
      virtual void addEntry() = 0;

      /** Remove all stale entries.
          Scans the table for entries that are no longer valid (i.e. they are
          stale, according to operation::isEntryStale) and removes them. This
          can be a time-consuming process (proportional to the number of cached
          entries).
      */
      virtual void removeStales() = 0;

      /** Removes all entries.
      */
      virtual void removeAll() = 0;

      /// Get performance stats for the table.
      const stats& getStats();

      /// For debugging.
      virtual void show(output &s, int verbLevel = 0) = 0;

    protected:
      stats perf;
};

// ******************************************************************
// *                                                                *
// *                        operation  class                        *
// *                                                                *
// ******************************************************************

/** Generic operation.
    Operations are tied to specific forests.
    Necessary for compute table entries.
*/
class MEDDLY::operation {
    const opname* theOpName;
    bool is_marked_for_deletion;
    int oplist_index;
    int key_length;
    int ans_length;
    /// List of free search_keys
    compute_table::search_key* CT_free_keys;

    // declared and initialized in meddly.cc
    static compute_table* Monolithic_CT;
    // declared and initialized in meddly.cc
    static operation** op_list;
    // declared and initialized in meddly.cc
    static int* op_holes;
    // declared and initialized in meddly.cc
    static int list_size;
    // declared and initialized in meddly.cc
    static int list_alloc;
    // declared and initialized in meddly.cc
    static int free_list;

    // should ONLY be called during library cleanup.
    static void destroyAllOps();

  protected:
    /// Compute table to use, if any.
    compute_table* CT;
    /// Struct for CT searches.
    // compute_table::search_key* CTsrch;
    // for cache of operations.
    operation* next;
    // must stale compute table hits be discarded.
    // if the result forest is using pessimistic deletion, then true.
    // otherwise, false.  MUST BE SET BY DERIVED CLASSES.
    bool discardStaleHits;

    virtual ~operation();
    void setAnswerForest(const expert_forest* f);
    void markForDeletion();
    void registerInForest(forest* f);
    void unregisterInForest(forest* f);
    virtual bool isStaleEntry(const node_handle* entry) = 0;
    compute_table::search_key* useCTkey();
    void allocEntryForests(int nf);
    void addEntryForest(int index, expert_forest* f);
    void allocEntryObjects(int no);
    void addEntryObject(int index);

    virtual bool checkForestCompatibility() const = 0;

    friend class forest;
    friend void MEDDLY::destroyOpInternal(operation* op);
    friend void MEDDLY::cleanup();

    friend class ct_initializer;

  public:
    /// New constructor.
    /// @param  n   Operation "name"
    /// @param  kl  Key length of compute table entries.
    ///             Use 0 if this operation does not use the compute table.
    /// @param  al  Answer length of compute table entries.
    ///             Use 0 if this operation does not use the compute table.
    operation(const opname* n, int kl, int al);

    bool isMarkedForDeletion() const;
    void setNext(operation* n);
    operation* getNext();

    static bool usesMonolithicComputeTable();
    static void removeStalesFromMonolithic();
    static void removeAllFromMonolithic();

    /// Remove stale compute table entries for this operation.
    void removeStaleComputeTableEntries();

    /// Remove all compute table entries for this operation.
    void removeAllComputeTableEntries();

    // for compute tables.

    int getIndex() const;
    static operation* getOpWithIndex(int i);
    static int getOpListSize();

    // for debugging:

    static void showMonolithicComputeTable(output &, int verbLevel);
    static void showAllComputeTables(output &, int verbLevel);
    void showComputeTable(output &, int verbLevel) const;

    // handy
    const char* getName() const;
    const opname* getOpName() const;

    /// Number of ints that make up the key (usually the operands).
    int getKeyLength() const;

    /// Number of ints that make up the answer (usually the results).
    int getAnsLength() const;

    /// Number of ints that make up the entire record (key + answer)
    int getCacheEntryLength() const;

    /// Checks if the cache entry (in entryData[]) is stale.
    bool isEntryStale(const node_handle* data);

    void doneCTkey(compute_table::search_key* K);

    /// Removes the cache entry (in entryData[]) by informing the
    /// applicable forests that the nodes in this entry are being removed
    /// from the cache
    virtual void discardEntry(const node_handle* entryData) = 0;

    /// Prints a string representation of this cache entry on strm (stream).
    virtual void showEntry(output &strm, const node_handle *entryData) const = 0;

    bool shouldStaleCacheHitsBeDiscarded() const;
};

// ******************************************************************
// *                                                                *
// *                     unary_operation  class                     *
// *                                                                *
// ******************************************************************

/** Mechanism to apply a unary operation in a specific forest.
    Specific operations will be derived from this class.
*/
class MEDDLY::unary_operation : public operation {
  protected:
    expert_forest* argF;
    expert_forest* resF;
    opnd_type resultType;

    virtual ~unary_operation();

    virtual bool checkForestCompatibility() const;

  public:
    unary_operation(const unary_opname* code, int kl, int al,
      expert_forest* arg, expert_forest* res);

    unary_operation(const unary_opname* code, int kl, int al,
      expert_forest* arg, opnd_type res);

    bool matches(const expert_forest* arg, const expert_forest* res)
      const;

    bool matches(const expert_forest* arg, opnd_type res) const;

    // high-level front-ends
    virtual void compute(const dd_edge &arg, dd_edge &res);
    virtual void computeDDEdge(const dd_edge &arg, dd_edge &res);
    virtual void compute(const dd_edge &arg, long &res);
    virtual void compute(const dd_edge &arg, double &res);
    virtual void compute(const dd_edge &arg, ct_object &c);

    // TBD: low-level front-ends?
    // e.g.,
    // virtual int computeDD(int k, int p);
    // virtual void computeEvDD(int k, int v, int p, int &w, int &q);
};

// ******************************************************************
// *                                                                *
// *                     binary_operation class                     *
// *                                                                *
// ******************************************************************

/** Mechanism to apply a binary operation in a specific forest.
    Specific operations will be derived from this class.
*/
class MEDDLY::binary_operation : public operation {
  protected:
    bool can_commute;
    expert_forest* arg1F;
    expert_forest* arg2F;
    expert_forest* resF;
    opnd_type resultType;

    virtual ~binary_operation();
    void operationCommutes();

    // Check if the variables orders of relevant forests are compatible
    virtual bool checkForestCompatibility() const;

  public:
    binary_operation(const binary_opname* code, int kl, int al,
      expert_forest* arg1, expert_forest* arg2, expert_forest* res);

    bool matches(const expert_forest* arg1, const expert_forest* arg2,
      const expert_forest* res) const;

    // high-level front-end
    virtual void compute(const dd_edge &ar1, const dd_edge &ar2, dd_edge &res);
    virtual void computeDDEdge(const dd_edge &ar1, const dd_edge &ar2, dd_edge &res)
      = 0;

    // low-level front ends

    /// Low-level compute on nodes a and b, return result.
    virtual node_handle compute(node_handle a, node_handle b);
    /// Low-level compute at level k on nodes a and b, return result.
    virtual node_handle compute(int k, node_handle a, node_handle b);

    /// Low-level compute on EV edges (av, ap) and (bv, bp), return result.
    virtual void compute(int av, node_handle ap, int bv, node_handle bp,
      int &cv, node_handle &cp);

    /// Low-level compute on EV edges (av, ap) and (bv, bp), return result.
    virtual void compute(float av, node_handle ap, float bv, node_handle bp,
      float &cv, node_handle &cp);

};

// ******************************************************************
// *                                                                *
// *                  specialized_operation  class                  *
// *                                                                *
// ******************************************************************

/** Mechanism to apply specialized operations.
*/
class MEDDLY::specialized_operation : public operation {
  public:
    specialized_operation(const specialized_opname* code, int kl, int al);
  protected:
    virtual ~specialized_operation();
  public:

    /** For unary (like) operations.
        Note that there could be other "built in" operands.
        Default behavior is to throw an exception.
    */
    virtual void compute(const dd_edge &arg, dd_edge &res);

    /** For binary (like) operations.
        Note that there could be other "built in" operands.
        Default behavior is to throw an exception.
    */
    virtual void compute(const dd_edge &ar1, const dd_edge &ar2, dd_edge &res);

    /** For numerical operations.
        compute y += some function of x, depending on the operation.
        Default behavior is to throw an exception.
    */
    virtual void compute(double* y, const double* x);
};

// ******************************************************************
// *                                                                *
// *                    global_rebuilder  class                     *
// *                                                                *
// ******************************************************************

/** Rebuild the dd_edge from the source forest in the target forest.
    The source and target forests may have different variable orders.
    While rebuilding, extra nodes may be created in the source forest
    because of the restrict operation.
*/

class MEDDLY::global_rebuilder {
private:
  struct RestrictKey {
    node_handle p;
    int var;
    int idx;

    bool operator==(const RestrictKey &other) const {
      return (p == other.p && var == other.var && idx == other.idx);
    }
  };

  struct RestrictKeyHasher {
    size_t operator()(const RestrictKey &key) const;
  };

  struct TransformKey {
    int sig;
//    int var;

    bool operator==(const TransformKey &other) const {
      return sig == other.sig;
//      return (sig == other.sig && var == other.var);
    }
  };

  struct TransformEntry {
    // Partial assignment
    std::vector<int> pa;
    node_handle p;

    bool operator==(const TransformEntry &other) const {
      return p == other.p;
    }
  };

  struct TransformKeyHasher {
    size_t operator()(const TransformKey &key) const;
  };

  class SignatureGenerator {
  protected:
    global_rebuilder &_gr;

  public:
    SignatureGenerator(global_rebuilder& gr);
    virtual void precompute() = 0;
    virtual int signature(node_handle p) = 0;
  };

  class TopDownSignatureGenerator: public SignatureGenerator {
  public:
    TopDownSignatureGenerator(global_rebuilder& gr);
    void precompute() override;
    int signature(node_handle p) override;
  };

  class BottomUpSignatureGenerator: public SignatureGenerator {
  private:
    std::unordered_map<node_handle, int> _cache_sig;
    std::unordered_map<node_handle, int> _cache_rec_sig;

    int rec_signature(node_handle p);

  public:
    BottomUpSignatureGenerator(global_rebuilder& gr);
    void precompute() override;
    int signature(node_handle p) override;
  };

  std::unordered_map<RestrictKey, node_handle, RestrictKeyHasher> _computed_restrict;
  std::unordered_multimap<TransformKey, TransformEntry, TransformKeyHasher> _computed_transform;
  SignatureGenerator* _sg;

  expert_forest* _source;
  expert_forest* _target;
  node_handle _root;
  int _hit;
  int _total;

  node_handle transform(node_handle p, int target_level, std::vector<int>& pa);
  node_handle restrict(node_handle p, std::vector<int>& pa);

  bool restrict_exist(node_handle p, const std::vector<int>& pa, int start,
      node_handle& result);
  int signature(node_handle p) const;

  // Return the top variable in the sub-order of the target variable order
  // starting from 0 to target_level
  // such that the given decision diagram depends on it.
  int check_dependency(node_handle p, int target_level) const;

public:
  friend class SignatureGenerator;

  global_rebuilder(expert_forest* source, expert_forest* target);
  ~global_rebuilder();

  dd_edge rebuild(const dd_edge& e);
  void clearCache();
  double hitRate() const;
};

#include "meddly_expert.hh"
#endif
