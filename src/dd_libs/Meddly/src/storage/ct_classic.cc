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


#include "ct_classic.h"

#include <map>  // for operation_map
#include <limits.h>

// #define DEBUG_SLOW
// #define DEBUG_CT
// #define DEBUG_TABLE2LIST
// #define DEBUG_LIST2TABLE
// #define DEBUG_CTALLOC

// #define DEBUG_REMOVESTALES
// #define SUMMARY_STALES


namespace MEDDLY {
  /// base class for all compute tables here;
  /// handles allocation of entries :^)
  class base_table;

  /// base class for hash tables (gives hash function)
  class base_hash;

  class base_chained;
  class base_unchained;

  class monolithic_chained;
  class operation_chained;

  class monolithic_unchained;
  class operation_unchained;

  class base_map;
}



// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                          base_table class                          *
// *                                                                    *
// *                                                                    *
// **********************************************************************

class MEDDLY::base_table : public compute_table {
  protected:
    class old_search_key : public compute_table::search_key {
        friend class MEDDLY::base_table;
        unsigned hash_value;
        node_handle* data;
        bool killData;
        node_handle* key_data;
        int currslot;
#ifdef DEVELOPMENT_CODE
        /// used only for range checking during "development".
        int keyLength;  
        bool has_hash;
#endif
      public:
        old_search_key(operation* op);
        virtual ~old_search_key();

        virtual void reset() {
          currslot = 0;
#ifdef DEVELOPMENT_CODE
          has_hash = false;
#endif
        }

        virtual void writeNH(node_handle nh) {
          MEDDLY_CHECK_RANGE(0, currslot, keyLength);
          key_data[currslot++] = nh;
        }
        virtual void write(int i) {
          MEDDLY_CHECK_RANGE(0, currslot, keyLength);
          key_data[currslot++] = i;
        }
        virtual void write(float f) {
          MEDDLY_CHECK_RANGE(0, currslot, keyLength);
          float* x = (float*) (key_data+currslot);
          x[0] = f;
          currslot++;
        }

        inline node_handle* rawData() const { return data; }
        // inline int dataLength() const { return hashLength; }
        inline int dataLength() const { return currslot + (key_data - data);}

        inline void setHash(unsigned h) {
          hash_value = h;
#ifdef DEVELOPMENT_CODE
          has_hash = true;
#endif
        }
        inline unsigned getHash() const {
          MEDDLY_DCASSERT(has_hash);
          return hash_value;
        }
    };

    class old_search_result : public compute_table::search_result {
        const node_handle* data;
        unsigned currslot;
#ifdef DEVELOPMENT_CODE
        unsigned ansLength;
#endif
      public:
        old_search_result() {
          data = 0;
        }
        virtual ~old_search_result() {
        }

        virtual node_handle readNH() {
          MEDDLY_DCASSERT(currslot < ansLength);
          return data[currslot++];
        }
        virtual void read(int &i) {
          MEDDLY_DCASSERT(currslot < ansLength);
          i = data[currslot++];
        }
        virtual void read(float &f) {
          MEDDLY_DCASSERT(currslot < ansLength);
          f = ((float*)(data + currslot))[0];
          currslot++;
        }
        virtual void read(long &L) {
          MEDDLY_DCASSERT( 
            currslot+sizeof(long)/sizeof(node_handle) <= ansLength);
          memcpy(&L, data+currslot, sizeof(long));
          currslot += sizeof(long) / sizeof(node_handle);
        }
        virtual void read(double &D) {
          MEDDLY_DCASSERT( 
            currslot+sizeof(double)/sizeof(node_handle) <= ansLength);
          memcpy(&D, data+currslot, sizeof(double));
          currslot += sizeof(double) / sizeof(node_handle);
        }
        virtual void read(void* &P) {
          MEDDLY_DCASSERT( 
            currslot+sizeof(void*)/sizeof(node_handle) <= ansLength);
          memcpy(&P, data+currslot, sizeof(void*));
          currslot += sizeof(void*) / sizeof(node_handle);
        }

        inline void setResult(const node_handle* d, int sz) {
          setValid();
          data = d;
          currslot = 0;
#ifdef DEVELOPMENT_CODE
          ansLength = sz;
#endif
        }

    };



    class old_temp_entry : public compute_table::entry_builder {
          friend class MEDDLY::base_table;
          int handle;
          unsigned hash_value;
#ifdef DEVELOPMENT_CODE
          bool has_hash_value;
#endif
          node_handle* entry;
          node_handle* res_entry;
          unsigned resSlot;
          // The remaining entries are used only in development code
#ifdef DEVELOPMENT_CODE
          unsigned resLength;
#endif
        public:
          old_temp_entry() { }
          virtual ~old_temp_entry() { }

          virtual void writeResultNH(node_handle nh)
          {
            MEDDLY_DCASSERT(resSlot < resLength);
            res_entry[resSlot++] = nh;
          }
          virtual void writeResult(int i)
          {
            MEDDLY_DCASSERT(resSlot < resLength);
            res_entry[resSlot++] = i;
          }
          virtual void writeResult(float f)
          {
            MEDDLY_DCASSERT(resSlot < resLength);
            float* x = (float*) res_entry + resSlot;
            x[0] = f;
            resSlot++;
          }
        protected:
          inline void writeResult(void* data, size_t slots)
          {
            MEDDLY_DCASSERT(slots>0);
            MEDDLY_DCASSERT(resSlot+slots<=resLength);
            memcpy(res_entry+resSlot, data, slots * sizeof(node_handle));
            resSlot += slots;
          }
        public:
          virtual void writeResult(long L)
          {
            writeResult(&L, sizeof(long) / sizeof(node_handle));
          }
          virtual void writeResult(double D)
          {
            writeResult(&D, sizeof(double) / sizeof(node_handle));
          }
          virtual void writeResult(void* P)
          {
            writeResult(&P, sizeof(void*) / sizeof(node_handle));
          }



          // The following are used by the compute table.
          inline const node_handle* readEntry(int off) const { return entry+off; }
          inline int readHandle() const { return handle; }
          inline unsigned getHash() const { 
            MEDDLY_DCASSERT(has_hash_value);
            return hash_value; 
          }
          inline node_handle& data(int i) {
            return entry[i];
          }
    };

  public:
    base_table(const ct_initializer::settings &s);
    virtual ~base_table();

  protected:
    old_temp_entry currEntry;
    

    inline void sawSearch(int c) {
      if (c>=stats::searchHistogramSize) {
        perf.numLargeSearches++;
      } else {
        perf.searchHistogram[c]++;
      }
      if (c>perf.maxSearchLength) perf.maxSearchLength = c;
    }
    int newEntry(int size);
    inline void recycleEntry(int h, int size) {
#ifdef DEBUG_CTALLOC
      fprintf(stderr, "Recycling entry %d size %d\n", h, size);
#endif
      entries[h] = freeList[size];
      freeList[size] = h;
      perf.numEntries--;
    }
    void dumpInternal(output &s, int verbLevel) const;
    void report(output &s, int indent, int &level) const;

    static inline bool equal_sw(const int* a, const int* b, int N) {
      switch (N) {  // note: cases 8 - 2 fall through
        case  8:    if (a[7] != b[7]) return false;
        case  7:    if (a[6] != b[6]) return false;
        case  6:    if (a[5] != b[5]) return false;
        case  5:    if (a[4] != b[4]) return false;
        case  4:    if (a[3] != b[3]) return false;
        case  3:    if (a[2] != b[2]) return false;
        case  2:    if (a[1] != b[1]) return false;
        case  1:    return a[0] == b[0];
        case  0:    return true;
        default:    return (0==memcmp(a, b, N*sizeof(int)));
      };
    }

    // M is 1 if we need a slot for the operation index, 0 otherwise.
    static inline search_key* init(operation* op, int M) {
      old_search_key* key = new old_search_key(op);
      MEDDLY_DCASSERT(0==key->data);
      key->data = new int[op->getKeyLength()+M];
      key->killData = true;
      key->key_data = key->data+M;
      if (M) key->data[0] = op->getIndex();
#ifdef DEVELOPMENT_CODE
      key->keyLength = op->getKeyLength();
#endif
      return key;
    }

    // C is the number of slots we need for "chaining".
    // M is 1 if we need a slot for the operation index, 0 otherwise.
    inline void startIndexedEntry(old_search_key* Key, int C, int M) {
      MEDDLY_DCASSERT(Key);
      operation* op = Key->getOp();
      MEDDLY_DCASSERT(op);
      currEntry.hash_value = Key->getHash();
      currEntry.resSlot = 0;
      currEntry.handle = newEntry(op->getCacheEntryLength()+C+M);
      currEntry.entry = entries + currEntry.handle;
      memcpy(currEntry.entry+C, Key->rawData(), Key->dataLength()*sizeof(node_handle));
      currEntry.res_entry = currEntry.entry + Key->dataLength() + C;
#ifdef DEVELOPMENT_CODE
      currEntry.resLength = op->getAnsLength();
      currEntry.has_hash_value = true;
#endif
      op->doneCTkey(Key);
    }

    // C is the number of slots we need for "chaining".
    // M is 1 if we need a slot for the operation index, 0 otherwise.
    inline int* startPtrEntry(old_search_key* Key, int C, int M) {
      MEDDLY_DCASSERT(Key);
      operation* op = Key->getOp();
      MEDDLY_DCASSERT(op);
      currEntry.resSlot = 0;
      int* data = new int[op->getCacheEntryLength()+M+C];
      currEntry.entry = data;
      memcpy(currEntry.entry+C, Key->rawData(), Key->dataLength()*sizeof(node_handle));
      currEntry.res_entry = currEntry.entry + Key->dataLength() + C;
#ifdef DEVELOPMENT_CODE
      currEntry.resLength = op->getAnsLength();
      currEntry.has_hash_value = false;
#endif
      op->doneCTkey(Key);
      return data;
    }
    
  protected:
    int*  entries;
    int entriesSize;
    int entriesAlloc;

    unsigned long currMemory;
    unsigned long peakMemory;
  private:
    static const int maxEntrySize = 15;
    static const int maxEntryBytes = sizeof(int) * maxEntrySize;
    int* freeList;
};



// **********************************************************************
// *                                                                    *
// *                 base_table::old_search_key methods                 *
// *                                                                    *
// **********************************************************************

MEDDLY::base_table::old_search_key::old_search_key(operation* op)
 : search_key(op)
{
  // hashLength = 0;
  data = 0;
  key_data = 0;
  killData = false;
}

MEDDLY::base_table::old_search_key::~old_search_key()
{
  if (killData) delete[] data;
}

// **********************************************************************
// *                                                                    *
// *                         base_table methods                         *
// *                                                                    *
// **********************************************************************

MEDDLY::base_table::base_table(const ct_initializer::settings &s)
 : compute_table(s)
{
  entriesAlloc = 1024;
  entries = (int*) malloc(entriesAlloc * sizeof(int));
  entriesSize = 1;
  // entries[0] is never, ever, used.
  if (0==entries) throw error(error::INSUFFICIENT_MEMORY);
  // for recycling entries
  freeList = new int[1+maxEntrySize];
  for (int i=0; i<=maxEntrySize; i++) freeList[i] = 0;

  currMemory = entriesAlloc * sizeof(int) + (1+maxEntrySize) * sizeof(int);
  peakMemory = currMemory;
}

MEDDLY::base_table::~base_table()
{
  free(entries);
  delete[] freeList;
}

int MEDDLY::base_table::newEntry(int size)
{
  // check free list
  if (size > maxEntrySize) {
    fprintf(stderr, "MEDDLY error: request for compute table entry larger than max size\n");
    throw error(error::MISCELLANEOUS);  // best we can do
  }
  if (size<1) return 0;
  perf.numEntries++;
  if (freeList[size]) {
    int h = freeList[size];
    freeList[size] = entries[h];
#ifdef DEBUG_CTALLOC
    fprintf(stderr, "Re-used entry %d size %d\n", h, size);
#endif
    return h;
  }
  if (entriesSize + size > entriesAlloc) {
    // Expand by a factor of 1.5
    int neA = entriesAlloc + (entriesAlloc/2);
    int* ne = (int*) realloc(entries, neA * sizeof(int));
    if (0==ne) {
      fprintf(stderr,
          "Error in allocating array of size %lu at %s, line %d\n",
          neA * sizeof(int), __FILE__, __LINE__);
      throw error(error::INSUFFICIENT_MEMORY);
    }
    currMemory += (neA - entriesAlloc) * sizeof(int);
    if (currMemory > peakMemory) peakMemory = currMemory;
    entries = ne;
    entriesAlloc = neA;
  }
  MEDDLY_DCASSERT(entriesSize + size <= entriesAlloc);
  int h = entriesSize;
  entriesSize += size;
#ifdef DEBUG_CTALLOC
  fprintf(stderr, "New entry %d size %d\n", h, size);
#endif
  return h;
}

void MEDDLY::base_table::dumpInternal(output &s, int verbLevel) const
{
  if (verbLevel < 1) return;
  if (0==entries) {
    s << "Entries: null\n";
  } else {
    s << "Entries: [" << long(entries[0]);
    for (int i=1; i<entriesSize; i++) {
      s << ", " << long(entries[i]);
    }
    s << "]\n";
  }
  if (0==freeList) {
    s << "Free: null\n";
  } else {
    s << "Free: [" << long(freeList[0]);
    for (int i=1; i<=maxEntrySize; i++) {
      s << ", " << long(freeList[i]);
    }
    s << "]\n";
  }
}

void MEDDLY::base_table
::report(output &s, int indent, int &level) const
{
  if (level < 1) return;
  s.put("", indent);
  s << "Number of entries :\t" << long(perf.numEntries) << "\n";
  s.put("", indent);
  s << "Entry array size  :\t" << long(entriesSize) << "\n";
  s.put("", indent);
  s << "Entry array alloc :\t" << long(entriesAlloc) << "\n";

  if (--level < 1) return;

  s.put("", indent);
  s << "Pings             :\t" << long(perf.pings) << "\n";
  s.put("", indent);
  s << "Hits              :\t" << long(perf.hits) << "\n";

  if (--level < 1) return;

  s.put("", indent);
  s << "Search length histogram:\n";
  for (int i=0; i<stats::searchHistogramSize; i++) {
    if (perf.searchHistogram[i]) {
      s.put("", indent+4);
      s.put(long(i), 3);
      s << ": " << long(perf.searchHistogram[i]) << "\n";
    }
  }
  if (perf.numLargeSearches) {
    s.put("", indent);
    s << "Searches longer than " << long(stats::searchHistogramSize-1)
      << ": " << long(perf.numLargeSearches) << "\n";
  }
  s.put("", indent);
  s << "Max search length: " << long(perf.maxSearchLength) << "\n";
}

// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                          base_hash  class                          *
// *                                                                    *
// *                                                                    *
// **********************************************************************

/** Abstract base class for hashing compute tables.
  * All use the same hash function:
  *
  *   Bob Jenkin's Hash
  *   Free to use for educational or commerical purposes
  *   http://burtleburtle.net/bob/hash/doobs.html
  */
class MEDDLY::base_hash : public base_table {
  public:
    base_hash(const ct_initializer::settings &s, int initTsz, 
      int initTex);
    virtual ~base_hash();

  protected:
    inline unsigned hash(const int* k, int length) const {
      return raw_hash(k, length) % tableSize;
    }
    inline unsigned hash(search_key* k) const {
      old_search_key* key = smart_cast<old_search_key*>(k);
      MEDDLY_DCASSERT(key);
      key->setHash(raw_hash(key->rawData(), key->dataLength()));
      return key->getHash() % tableSize;
    }
    
    void dumpInternal(output &s, int verbLevel) const;
    void report(output &s, int indent, int &level) const;
  protected:
    int*  table;
    unsigned int tableSize;
    unsigned int tableExpand;
    unsigned int tableShrink;
  private:
    static inline unsigned rot(unsigned x, int k) {
        return (((x)<<(k)) | ((x)>>(32-(k))));
    }
    static inline void mix(unsigned &a, unsigned &b, unsigned &c) {
        a -= c;  a ^= rot(c, 4);  c += b; 
        b -= a;  b ^= rot(a, 6);  a += c;
        c -= b;  c ^= rot(b, 8);  b += a;
        a -= c;  a ^= rot(c,16);  c += b;
        b -= a;  b ^= rot(a,19);  a += c;
        c -= b;  c ^= rot(b, 4);  b += a;
    }
    static inline void final(unsigned &a, unsigned &b, unsigned &c) {
        c ^= b; c -= rot(b,14);
        a ^= c; a -= rot(c,11);
        b ^= a; b -= rot(a,25);
        c ^= b; c -= rot(b,16);
        a ^= c; a -= rot(c,4); 
        b ^= a; b -= rot(a,14);
        c ^= b; c -= rot(b,24);
    }
    static unsigned raw_hash(const int* k, int length);
};


// **********************************************************************
// *                                                                    *
// *                         base_hash  methods                         *
// *                                                                    *
// **********************************************************************

MEDDLY::base_hash::base_hash(const ct_initializer::settings &s, 
  int initTsz, int initTex) : base_table(s)
{
  tableSize = initTsz;
  tableExpand = initTex;
  tableShrink = 0;
  table = (int*) malloc(tableSize * sizeof(int));
  if (0==table) throw error(error::INSUFFICIENT_MEMORY);
  for (unsigned i=0; i<tableSize; i++) table[i] = 0;

  currMemory += tableSize * sizeof(int);
  peakMemory = currMemory;
}

MEDDLY::base_hash::~base_hash()
{
  free(table);
}

void MEDDLY::base_hash::dumpInternal(output &s, int verbLevel) const
{
  if (verbLevel < 1) return;
  if (0==table) {
    s << "Table: null\n";
  } else {
    s << "Table: [" << long(table[0]);
    for (unsigned i=1; i<tableSize; i++) {
      s << ", " << long(table[i]);
    }
    s << "]\n";
  }
  base_table::dumpInternal(s, verbLevel-1);
}

void MEDDLY::base_hash
::report(output &s, int indent, int &level) const
{
  if (level < 1) return;
  s.put("", indent);
  s << "Hash table size   :\t" << long(tableSize) << "\n";
  base_table::report(s, indent, level);
}

unsigned MEDDLY::base_hash::raw_hash(const int* k, int length)
{
  unsigned a, b, c;
  a = b = c = 0xdeadbeef;

  // handle most of the key
  while (length > 3)
  {
    a += *k++;
    b += *k++;
    c += *k++;
    mix(a,b,c);
    length -= 3;
  }

  // handle the last 3 uint32_t's
  switch(length)
  { 
    // all the case statements fall through
    case 3: c += k[2];
    case 2: b += k[1];
    case 1: a += k[0];
            final(a,b,c);
    case 0: // nothing left to add
            break;
  }

  return c;
}



// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                         base_chained class                         *
// *                                                                    *
// *                                                                    *
// **********************************************************************

/// Abstract base class for hash tables with chaining.
class MEDDLY::base_chained : public base_hash {
  public:
    base_chained(const ct_initializer::settings &s);
    virtual ~base_chained();

    virtual void addEntry();
    virtual void removeStales();
  protected:
    void dumpInternal(output &s, int verbLevel) const;
    virtual int convertToList(bool removeStales) = 0;
    virtual void listToTable(int h) = 0;
    virtual void showEntry(output &s, int h) const = 0;
};



// **********************************************************************
// *                                                                    *
// *                        base_chained methods                        *
// *                                                                    *
// **********************************************************************

MEDDLY::base_chained::base_chained(const ct_initializer::settings &s)
 : base_hash(s, 1024, 4*1024)
{
}

MEDDLY::base_chained::~base_chained()
{
}

void MEDDLY::base_chained::addEntry()
{
  unsigned h = currEntry.getHash() % tableSize;
  currEntry.data(0) = table[h];
  table[h] = currEntry.readHandle();

#ifdef DEBUG_CT
  printf("Adding CT entry ");
  showEntry(stdout, currEntry.readHandle());
  // fprintf(stderr, " to slot %u", h);
  printf("\n");
#endif

  if (perf.numEntries < tableExpand) return;

#ifdef DEBUG_SLOW
  fprintf(stdout, "Running GC in compute table (size %d, entries %ld)\n", 
    tableSize, perf.numEntries
  );
#endif

  int list = convertToList(checkStalesOnResize);
  if (perf.numEntries < tableSize) {
    // Don't need to expand
    listToTable(list);
#ifdef DEBUG_SLOW
    fprintf(stdout, "Done CT GC, no resizing (now entries %ld)\n", 
      perf.numEntries
    );
#endif
    return;
  } 

  unsigned newsize = tableSize*2;
  if (newsize > maxSize) newsize = maxSize;

  int* newt = (int*) realloc(table, newsize * sizeof(int));
  if (0==newt) {
    fprintf(stderr,
        "Error in allocating array of size %lu at %s, line %d\n",
        newsize * sizeof(int), __FILE__, __LINE__);
    throw error(error::INSUFFICIENT_MEMORY);
  }

  for (unsigned i=tableSize; i<newsize; i++) newt[i] = 0;

  currMemory += (newsize - tableSize) * sizeof(int);
  if (currMemory > peakMemory) peakMemory = currMemory;

  table = newt;
  tableSize = newsize;
  if (tableSize == maxSize) {
    tableExpand = INT_MAX;
  } else {
    tableExpand = 4*tableSize;
  }
  tableShrink = tableSize / 2;

  listToTable(list);
#ifdef DEBUG_SLOW
  fprintf(stdout, "CT enlarged to size %d\n", tableSize);
#endif
}

void MEDDLY::base_chained::removeStales()
{
#ifdef DEBUG_SLOW
  fprintf(stdout, "Removing stales in CT (size %d, entries %ld)\n", 
    tableSize, perf.numEntries
  );
#endif
  int list = convertToList(true);
  if (perf.numEntries < tableShrink) {
    // shrink table
    int newsize = tableSize / 2;
    if (newsize < 1024) newsize = 1024;
    int* newt = (int*) realloc(table, newsize * sizeof(int));
    if (0==newt) {
      fprintf(stderr,
          "Error in allocating array of size %lu at %s, line %d\n",
          newsize * sizeof(int), __FILE__, __LINE__);
      throw error(error::INSUFFICIENT_MEMORY); 
    }

    currMemory -= (tableSize - newsize) * sizeof(int);  

    table = newt;
    tableSize = newsize;
    tableExpand = 4*tableSize;
    if (1024 == tableSize) {
      tableShrink = 0;
    } else {
      tableShrink = tableSize / 2;
    }
  }
  listToTable(list);
#ifdef DEBUG_SLOW
  fprintf(stdout, "Done removing CT stales (size %d, entries %ld)\n", 
    tableSize, perf.numEntries
  );
#endif
}

void MEDDLY::base_chained::dumpInternal(output &s, int verbLevel) const
{
  if (verbLevel < 1) return;

  s << "Hash table chains:\n";

  for (unsigned i=0; i<tableSize; i++) {
    if (0==table[i]) continue;
    s << "table[";
    s.put(long(i), 9);
    s << "]: " << long(table[i]);
    int curr = entries[table[i]];
    while (curr) {
      s << " -> " << long(curr);
      curr = entries[curr];
    }
    s.put('\n');
  }

  s << "\nHash table nodes:\n";
  
  for (unsigned i=0; i<tableSize; i++) {
    int curr = table[i];
    while (curr) {
      s << "\tNode ";
      s.put(long(curr), 9);
      s << ":  ";
      showEntry(s, curr);
      s.put('\n');
      curr = entries[curr];
    }
  }
  s.put('\n');

  base_hash::dumpInternal(s, verbLevel-1);
}


// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                      monolithic_chained class                      *
// *                                                                    *
// *                                                                    *
// **********************************************************************

/*
    Anatomy of an entry:

      entries[h]      : next pointer in chain
      entries[h+1]    : operation index
      entries[h+2]    : first "payload" item
      ...
      entries[h+L+1]  : last "payload" item, L is cache entry length.
*/
class MEDDLY::monolithic_chained : public base_chained {
  public:
    monolithic_chained(const ct_initializer::settings &s);
    virtual ~monolithic_chained();

    // required functions

    virtual bool isOperationTable() const   { return false; }
    virtual search_key* initializeSearchKey(operation* op);
    virtual search_result& find(search_key *key);
    virtual entry_builder& startNewEntry(search_key *key);
    virtual void removeAll();

    virtual void show(output &s, int verbLevel);

  protected:
    virtual int convertToList(bool removeStales);
    virtual void listToTable(int h);
    virtual void showEntry(output &s, int h) const;

    inline bool checkStale(unsigned h, int prev, int &curr) {
        operation* currop = operation::getOpWithIndex(entries[curr+1]);
        MEDDLY_DCASSERT(currop);
        if (currop->isEntryStale(entries+curr+2)) {
          currop->discardEntry(entries+curr+2);
          int next = entries[curr];
          if (prev) entries[prev] = next;
          else      table[h] = next;
          int length = currop->getCacheEntryLength();
          recycleEntry(curr, length+2);
          curr = next;
          return true;
        }
        return false;
    }
};



// **********************************************************************
// *                                                                    *
// *                     monolithic_chained methods                     *
// *                                                                    *
// **********************************************************************

MEDDLY::monolithic_chained
::monolithic_chained(const ct_initializer::settings &s)
 : base_chained(s)
{
}

MEDDLY::monolithic_chained::~monolithic_chained()
{
}

MEDDLY::compute_table::search_key* 
MEDDLY::monolithic_chained::initializeSearchKey(operation* op)
{
  return init(op, 1);
}


MEDDLY::compute_table::search_result& 
MEDDLY::monolithic_chained::find(search_key *k)
{
  static old_search_result ANS;
  old_search_key* key = smart_cast <old_search_key*>(k);
  MEDDLY_DCASSERT(key);
  perf.pings++;
  unsigned h = hash(key);
  int prev = 0;
  int curr = table[h];
  int chain = 0;
  ANS.setInvalid();
  while (curr) {
    chain++;
    //
    // Check for match
    //
    if (equal_sw(entries+curr+1, key->rawData(), key->dataLength())) {
      if (key->getOp()->shouldStaleCacheHitsBeDiscarded()) {
        if (checkStale(h, prev, curr)) {
          // The match is stale.
          // Since there can NEVER be more than one match
          // in the table, we're done!
          break;
        }
      }
      // "Hit"
      perf.hits++;
      if (prev) {
        // not at the front; move it there
        entries[prev] = entries[curr];
        entries[curr] = table[h];
        table[h] = curr;
      }
#ifdef DEBUG_CT
      printf("Found CT entry ");
      key->getOp()->showEntry(stdout, entries + curr + 2);
      // fprintf(stderr, " in slot %u", h);
      printf("\n");
#endif
      ANS.setResult(entries+curr+1+key->dataLength(), key->getOp()->getAnsLength());
      break;
    };
    //
    // No match; maybe check stale
    //
    if (checkStalesOnFind) {
      if (checkStale(h, prev, curr)) continue;
    }
    // advance pointers
    prev = curr;
    curr = entries[curr];
  }
  sawSearch(chain);
  return ANS;
}


MEDDLY::compute_table::entry_builder& 
MEDDLY::monolithic_chained::startNewEntry(search_key *key)
{
  MEDDLY_DCASSERT(key);
  startIndexedEntry(smart_cast<old_search_key*>(key), 1, 1);
  return currEntry;
}

void MEDDLY::monolithic_chained::removeAll()
{
  for (unsigned i=0; i<tableSize; i++) {
    while (table[i]) {
      int curr = table[i];
      table[i] = entries[curr];
      operation* currop = operation::getOpWithIndex(entries[curr+1]);
      MEDDLY_DCASSERT(currop);
      currop->discardEntry(entries + curr + 2);
      recycleEntry(curr, 2+currop->getCacheEntryLength());
    } // while
  } // for i
}

void MEDDLY::monolithic_chained::show(output &s, int verbLevel) 
{
  if (verbLevel < 1) return;
  s << "Monolithic compute table\n";
  s.put("", 6);
  s << "Current CT memory :\t" << long(currMemory) << " bytes\n";
  s.put("", 6);
  s << "Peak    CT memory :\t" << long(peakMemory) << " bytes\n";
  // verbLevel--;
  report(s, 6, verbLevel);
  verbLevel--;
  dumpInternal(s, verbLevel);
}


int MEDDLY::monolithic_chained::convertToList(bool removeStales)
{
  int list = 0;
  for (unsigned i=0; i<tableSize; i++) {
    while (table[i]) {
      int curr = table[i];
      table[i] = entries[curr];
      if (removeStales) {
        operation* currop = operation::getOpWithIndex(entries[curr+1]);
        MEDDLY_DCASSERT(currop);
        const int* entry = entries + curr + 2;
        //
        // Check for stale
        //
        if (currop->isEntryStale(entry)) {
#ifdef DEBUG_TABLE2LIST
          printf("\tstale ");
          currop->showEntry(stdout, entry);
          printf(" (handle %d slot %d)\n", curr, i);
#endif
          currop->discardEntry(entry);
          recycleEntry(curr, 2+currop->getCacheEntryLength());
          continue;
        }
      } // if removeStales
      //
      // Not stale, move to list
      //
#ifdef DEBUG_TABLE2LIST
      printf("\tkeep  ");
      currop->showEntry(stdout, entry);
      printf(" (handle %d slot %d)\n", curr, i);
#endif
      entries[curr] = list;
      list = curr;
    } // while
  } // for i
#ifdef DEBUG_TABLE2LIST
  printf("Built list: %d", list);
  if (list ) for (int L=entries[list]; L; L=entries[L])
    printf("->%d", L);
  printf("\n");
#endif
  return list;
}

void MEDDLY::monolithic_chained::listToTable(int L)
{
#ifdef DEBUG_LIST2TABLE
  printf("Recovering  list: %d", L);
  if (L) for (int i=entries[L]; i; i=entries[i])
    printf("->%d", i);
  printf("\n");
#endif
  while (L) {
    int curr = L;
    L = entries[L];
    operation* currop = operation::getOpWithIndex(entries[curr+1]);
    MEDDLY_DCASSERT(currop);
    int hashlength = 1+currop->getKeyLength();
    unsigned h = hash(entries + curr + 1, hashlength);
    entries[curr] = table[h];
    table[h] = curr;
#ifdef DEBUG_LIST2TABLE
    printf("\tsave  ");
    currop->showEntry(stdout, entries + curr + 2);
    printf(" (handle %d slot %d)\n", curr, h);
#endif
  }
}

void MEDDLY::monolithic_chained::showEntry(output &s, int curr) const
{ 
  operation* op = operation::getOpWithIndex(entries[curr+1]);
  op->showEntry(s, entries + curr + 2);
}


// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                      operation_chained  class                      *
// *                                                                    *
// *                                                                    *
// **********************************************************************

/*
    Anatomy of an entry:

      entries[h]      : next pointer in chain
      entries[h+1]    : first "payload" item
      ...
      entries[h+L]    : last "payload" item, L is cache entry length.
*/
class MEDDLY::operation_chained : public base_chained {
  public:
    operation_chained(const ct_initializer::settings &s, operation* op);
    virtual ~operation_chained();

    // required functions

    virtual bool isOperationTable() const   { return true; }
    virtual search_key* initializeSearchKey(operation* op);
    virtual entry_builder& startNewEntry(search_key *key);
    virtual void removeAll();

    virtual void show(output &s, int verbLevel);

  protected:
    virtual int convertToList(bool removeStales);
    virtual void listToTable(int h);
    virtual void showEntry(output &s, int h) const { 
      global_op->showEntry(s, entries + h + 1);
    }

    inline bool checkStale(unsigned h, int prev, int &curr) {
        if (global_op->isEntryStale(entries+curr+1)) {
          global_op->discardEntry(entries+curr+1);
          int next = entries[curr];
          if (prev) entries[prev] = next;
          else      table[h] = next;
          int length = global_op->getCacheEntryLength();
          recycleEntry(curr, length+1);
          curr = next;
          return true;
        }
        return false;
    }
  protected:
    operation* global_op;
};


// **********************************************************************
// *                                                                    *
// *                     operation_chained  methods                     *
// *                                                                    *
// **********************************************************************

MEDDLY::operation_chained
::operation_chained(const ct_initializer::settings &s, operation* op)
 : base_chained(s)
{
  global_op = op;
}

MEDDLY::operation_chained::~operation_chained()
{
}

MEDDLY::compute_table::search_key* 
MEDDLY::operation_chained::initializeSearchKey(operation* op)
{
  if (op != global_op)
    throw error(error::UNKNOWN_OPERATION);
  return init(op, 0);
}

MEDDLY::compute_table::entry_builder& 
MEDDLY::operation_chained::startNewEntry(search_key *key)
{
  MEDDLY_DCASSERT(key);
  if (key->getOp() != global_op)
    throw error(error::UNKNOWN_OPERATION);
  startIndexedEntry(smart_cast<old_search_key*>(key), 1, 0);
  return currEntry;
}

void MEDDLY::operation_chained::removeAll()
{
  for (unsigned i=0; i<tableSize; i++) {
    while (table[i]) {
      int curr = table[i];
      table[i] = entries[curr];
      global_op->discardEntry(entries + curr + 1);
      recycleEntry(curr, 1+global_op->getCacheEntryLength());
    } // while
  } // for i
}

void MEDDLY::operation_chained::show(output &s, int verbLevel)
{
  if (verbLevel < 1) return;
  s << "Compute table for " << global_op->getName() << " (index " 
    << long(global_op->getIndex()) << ")\n";
  s.put("", 6);
  s << "Current CT memory :\t" << long(currMemory) << " bytes\n";
  s.put("", 6);
  s << "Peak    CT memory :\t" << long(peakMemory) << " bytes\n";
  // verbLevel--;
  report(s, 6, verbLevel);
  verbLevel--;
  dumpInternal(s, verbLevel);
}


int MEDDLY::operation_chained::convertToList(bool removeStales)
{
  int list = 0;
  for (unsigned i=0; i<tableSize; i++) {
    while (table[i]) {
      int curr = table[i];
      table[i] = entries[curr];
      if (removeStales) {
        const int* entry = entries + curr + 1;
        //
        // Check for stale
        //
        if (global_op->isEntryStale(entry)) {
          global_op->discardEntry(entry);
          recycleEntry(curr, 1+global_op->getCacheEntryLength());
          continue;
        }
      }
      //
      // Not stale, move to list
      //
      entries[curr] = list;
      list = curr;
    } // while
  } // for i
  return list;
}

void MEDDLY::operation_chained::listToTable(int L)
{
  while (L) {
    int curr = L;
    L = entries[L];
    int hashlength = global_op->getKeyLength();
    int h = hash(entries + curr + 1, hashlength);
    entries[curr] = table[h];
    table[h] = curr;
  }
}

// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                    operation_chained_fast class                    *
// *                                                                    *
// *                                                                    *
// **********************************************************************

namespace MEDDLY {
  template <int N>
  class operation_chained_fast : public operation_chained {
    public:
      operation_chained_fast(const ct_initializer::settings &s, 
        operation* op) : operation_chained(s, op) { }
      virtual ~operation_chained_fast() { }
      virtual search_result& find(search_key *key);
  };
};


// **********************************************************************
// *                                                                    *
// *                   operation_chained_fast methods                   *
// *                                                                    *
// **********************************************************************


template <int N>
MEDDLY::compute_table::search_result& 
MEDDLY::operation_chained_fast<N>::find(search_key *k)
{
  static old_search_result ANS;
  old_search_key* key = smart_cast <old_search_key*>(k);
  MEDDLY_DCASSERT(key);
  perf.pings++;
  unsigned h = hash(key);
  int prev = 0;
  int curr = table[h];
  int chain = 0;
  ANS.setInvalid();
  while (curr) {
    chain++;
    //
    // Check for match
    //
    if (equal_sw(entries+curr+1, key->rawData(), N)) {
      if (global_op->shouldStaleCacheHitsBeDiscarded()) {
        if (checkStale(h, prev, curr)) {
          // The match is stale.
          // Since there can NEVER be more than one match
          // in the table, we're done!
          break;
        }
      } 
      // "Hit"
      perf.hits++;
      if (prev) {
        // not at the front; move it there
        entries[prev] = entries[curr];
        entries[curr] = table[h];
        table[h] = curr;
      }
#ifdef DEBUG_CT
      printf("Found CT entry ");
      global_op->showEntry(stdout, entries + curr + 1);
      printf("\n");
#endif
      ANS.setResult(entries+curr+1+key->dataLength(), global_op->getAnsLength());
      break;
    };
    //
    // No match; maybe check stale
    //
    if (checkStalesOnFind) {
      if (checkStale(h, prev, curr)) continue;
    }
    // advance pointers
    prev = curr;
    curr = entries[curr];
  }
  sawSearch(chain);
  return ANS;
}

// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                        base_unchained class                        *
// *                                                                    *
// *                                                                    *
// **********************************************************************

/// Abstract base class for hash tables without chaining.
class MEDDLY::base_unchained : public base_hash {
  public:
    base_unchained(const ct_initializer::settings &s);
    virtual ~base_unchained();

    virtual void show(output &s, int verbLevel);
  protected:
    virtual void showTitle(output &s) const = 0;
    virtual void showEntry(output &s, int h) const = 0;

    inline void incMod(unsigned &h) {
      h++;
      if (h>=tableSize) h=0;
    }
    template <int M>
    inline void remove(int curr) {
        operation* currop = M 
          ?   operation::getOpWithIndex(entries[curr])
          :   global_op;
        MEDDLY_DCASSERT(currop);
#ifdef DEBUG_CT
        printf("Removing CT entry ");
        currop->showEntry(stdout, entries+curr+M);
        printf("\n");
#endif  
        currop->discardEntry(entries+curr+M);
        int length = currop->getCacheEntryLength();
        recycleEntry(curr, length+M);
    }
    template <int M>
    inline void setTable(unsigned h, int curr) {
        unsigned hfree = h;
        for (int i=maxCollisionSearch; i>=0; i--, incMod(hfree)) {
          // find a free slot
          if (0==table[hfree]) {
            table[hfree] = curr;
            return;
          }
        }
        // full; remove entry at our slot.
        collisions++;    
        remove<M>(table[h]);
        table[h] = curr;
    }
    template <int M>
    inline bool checkStale(unsigned h, int curr) {
        operation* currop = M 
          ?   operation::getOpWithIndex(entries[curr])
          :   global_op;
        MEDDLY_DCASSERT(currop);
        if (currop->isEntryStale(entries+curr+M)) {
#ifdef DEBUG_CT
          printf("Removing CT stale entry ");
          currop->showEntry(stdout, entries+curr+M);
          printf("\n");
#endif  
          currop->discardEntry(entries+curr+M);
          table[h] = 0;
          int length = currop->getCacheEntryLength();
          recycleEntry(curr, length+M);
          return true;
        }
        return false;
    }
    template <int M>
    inline void scanForStales() {
      for (unsigned i=0; i<tableSize; i++) {
        if (0==table[i]) continue;
        checkStale<M>(i, table[i]);
      }
    }
    template <int M>
    inline void rehashTable(int* oldT, unsigned oldS) {
        for (unsigned i=0; i<oldS; i++) {
          int curr = oldT[i];
          if (0==curr) continue;
          operation* currop = M 
            ?   operation::getOpWithIndex(entries[curr])
            :   global_op;
          MEDDLY_DCASSERT(currop);
          int hashlength = M+currop->getKeyLength();
          unsigned h = hash(entries + curr, hashlength);
          setTable<M>(h, curr);
        }
    }
    template <int M>
    inline void addEntryT() {
      unsigned h = currEntry.getHash() % tableSize;

#ifdef DEBUG_CT
      printf("Adding CT entry ");
      showEntry(stdout, currEntry.readHandle());
      // fprintf(stderr, " to slot %u", h);
      printf("\n");
#endif

      setTable<M>(h, currEntry.readHandle());
  
      if (perf.numEntries < tableExpand) return;

#ifdef DEBUG_SLOW
      fprintf(stdout, "Running GC in compute table (size %d, entries %ld)\n", 
        tableSize, perf.numEntries
      );
#endif

      if (checkStalesOnResize) {
        scanForStales<M>();
        if (perf.numEntries < tableExpand / 4) {
#ifdef DEBUG_SLOW
          fprintf(stdout, "Done CT GC, no resizing (now entries %ld)\n", 
            perf.numEntries
          );
#endif
          return;
        }
      }

      unsigned newsize = tableSize*2;
      if (newsize > maxSize) newsize = maxSize;
      if (tableSize == newsize) return;

      int* oldT = table;
      unsigned oldSize = tableSize;
      tableSize = newsize;
      table = (int*) malloc(newsize * sizeof(int));
      if (0==table) {
        table = oldT;
        tableSize = oldSize;
        fprintf(stderr,
            "Error in allocating array of size %lu at %s, line %d\n",
            newsize * sizeof(int), __FILE__, __LINE__);
        throw error(error::INSUFFICIENT_MEMORY);
      }
      for (unsigned i=0; i<newsize; i++) table[i] = 0;

      currMemory += newsize * sizeof(int);

      rehashTable<M>(oldT, oldSize);
      free(oldT);

      currMemory -= oldSize * sizeof(int);
      if (currMemory > peakMemory) peakMemory = currMemory;

      if (tableSize == maxSize) {
        tableExpand = INT_MAX;
      } else {
        tableExpand = tableSize / 2;
      }
      tableShrink = tableSize / 8;

#ifdef DEBUG_SLOW
      fprintf(stdout, "CT enlarged to size %d\n", tableSize);
#endif
    }
    template <int M>
    inline void removeStalesT() {
#ifdef DEBUG_SLOW
      fprintf(stdout, "Removing stales in CT (size %d, entries %ld)\n", 
        tableSize, perf.numEntries
      );
#endif

      scanForStales<M>();

      if (perf.numEntries < tableShrink) {
        // shrink table
        unsigned newsize = tableSize / 2;
        if (newsize < 1024) newsize = 1024;
        if (newsize < tableSize) {
          int* oldT = table;
          unsigned oldSize = tableSize;
          tableSize = newsize;
          table = (int*) malloc(newsize * sizeof(int));
          if (0==table) {
            table = oldT;
            tableSize = oldSize;
            fprintf(stderr,
                "Error in allocating array of size %lu at %s, line %d\n",
                newsize * sizeof(int), __FILE__, __LINE__);
            throw error(error::INSUFFICIENT_MEMORY);
          }
          for (unsigned i=0; i<newsize; i++) table[i] = 0;
          currMemory += newsize * sizeof(int);
    
          rehashTable<M>(oldT, oldSize);
          free(oldT);
      
          currMemory -= oldSize * sizeof(int);
          if (currMemory > peakMemory) peakMemory = currMemory;
    
          tableExpand = tableSize / 2;
          if (1024 == tableSize) {
            tableShrink = 0;
          } else {
            tableShrink = tableSize / 8;
          }
        } // if different size
      }
    
#ifdef DEBUG_SLOW
      fprintf(stdout, "Done removing CT stales (size %d, entries %ld)\n", 
        tableSize, perf.numEntries
      );
#endif
    }


    
  protected:
    static const int maxCollisionSearch = 2;
    long collisions;
    operation* global_op;
};



// **********************************************************************
// *                                                                    *
// *                       base_unchained methods                       *
// *                                                                    *
// **********************************************************************

MEDDLY::base_unchained
::base_unchained(const ct_initializer::settings &s)
 : base_hash(s, 1024, 512)
{
  collisions = 0;
  global_op = 0;
}

MEDDLY::base_unchained::~base_unchained()
{
}

void MEDDLY::base_unchained::show(output &s, int verbLevel) 
{
  if (verbLevel < 1) return;
  showTitle(s);
  s.put("", 6);
  s << "Current CT memory :\t" << long(currMemory) << " bytes\n";
  s.put("", 6);
  s << "Peak    CT memory :\t" << long(peakMemory) << " bytes\n";
  // verbLevel--;
  // if (verbLevel < 1) return;
  s.put("", 6);
  s << "Collisions        :\t" << long(collisions) << "\n";
  report(s, 6, verbLevel);
  verbLevel--;
  if (verbLevel < 1) return;

  s << "\nHash table:\n";
  
  for (unsigned i=0; i<tableSize; i++) {
    int curr = table[i];
    if (0==curr) continue;
    s.put('\t');
    s.put(long(i), 9);
    s << ":  node ";
    s.put(long(curr), 9);
    s << ": ";
    showEntry(s, curr);
    s.put('\n');
  }

  base_hash::dumpInternal(s, verbLevel-1);
}


// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                     monolithic_unchained class                     *
// *                                                                    *
// *                                                                    *
// **********************************************************************

/*
    Anatomy of an entry:

      entries[h]      : operation index
      entries[h+1]    : first "payload" item
      ...
      entries[h+L]    : last "payload" item, L is cache entry length.
*/
class MEDDLY::monolithic_unchained : public base_unchained {
  public:
    monolithic_unchained(const ct_initializer::settings &s);
    virtual ~monolithic_unchained();

    // required functions

    virtual bool isOperationTable() const   { return false; }
    virtual search_key* initializeSearchKey(operation* op);
    virtual search_result& find(search_key *key);
    virtual entry_builder& startNewEntry(search_key *key);
    virtual void addEntry();
    virtual void removeStales();
    virtual void removeAll();
  protected:
    virtual void showTitle(output &s) const;
    virtual void showEntry(output &s, int curr) const;
};


// **********************************************************************
// *                                                                    *
// *                    monolithic_unchained methods                    *
// *                                                                    *
// **********************************************************************

MEDDLY::monolithic_unchained
::monolithic_unchained(const ct_initializer::settings &s)
 : base_unchained(s)
{
}

MEDDLY::monolithic_unchained::~monolithic_unchained()
{
}

MEDDLY::compute_table::search_key* 
MEDDLY::monolithic_unchained::initializeSearchKey(operation* op)
{
  return init(op, 1);
}


MEDDLY::compute_table::search_result&
MEDDLY::monolithic_unchained::find(search_key *k)
{
  static old_search_result ANS;
  old_search_key* key = smart_cast <old_search_key*>(k);
  MEDDLY_DCASSERT(key);
  perf.pings++;
  unsigned h = hash(key);
  unsigned hcurr = h;
  int chain;
  ANS.setInvalid();
  for (chain=0; chain<=maxCollisionSearch; chain++, incMod(hcurr) ) {
    int curr = table[hcurr];
    if (0==curr) continue;
    //
    // Check for match
    //
    if (equal_sw(entries+curr, key->rawData(), key->dataLength())) {
      if (key->getOp()->shouldStaleCacheHitsBeDiscarded()) {
        if (checkStale<1>(hcurr, curr)) {
          // The match is stale.
          // Since there can NEVER be more than one match
          // in the table, we're done!
          break;
        }
      }
      // "Hit"
      perf.hits++;
#ifdef DEBUG_CT
      printf("Found CT entry ");
      key->getOp()->showEntry(stdout, entries + curr + 1);
      // fprintf(stderr, " in slot %u", h);
      printf("\n");
#endif
      ANS.setResult(entries+curr+key->dataLength(), key->getOp()->getAnsLength());
      break;
    };
    //
    // No match; maybe check stale
    //
    if (checkStalesOnFind) {
      checkStale<1>(hcurr, curr);
    }
  } // for chain
  sawSearch(chain);
  return ANS;
}

MEDDLY::compute_table::entry_builder& 
MEDDLY::monolithic_unchained::startNewEntry(search_key *key)
{
  MEDDLY_DCASSERT(key);
  startIndexedEntry(smart_cast<old_search_key*>(key), 0, 1);
  return currEntry;
}

void MEDDLY::monolithic_unchained::addEntry()
{
  addEntryT<1>();
}

void MEDDLY::monolithic_unchained::removeStales()
{
  removeStalesT<1>();
}
   
void MEDDLY::monolithic_unchained::removeAll()
{
  for (unsigned i=0; i<tableSize; i++) {
    if (0==table[i]) continue;
    remove<1>(table[i]);
    table[i] = 0;
  }
}

void MEDDLY::monolithic_unchained::showTitle(output &s) const
{
  s << "Monolithic compute table\n";
}

void MEDDLY::monolithic_unchained::showEntry(output &s, int curr) const 
{ 
  operation* op = operation::getOpWithIndex(entries[curr]);
  op->showEntry(s, entries + curr + 1);
}

// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                     operation_unchained  class                     *
// *                                                                    *
// *                                                                    *
// **********************************************************************

/*
    Anatomy of an entry:

      entries[h]      : first "payload" item
      ...
      entries[h+L-1]  : last "payload" item, L is cache entry length.
*/
class MEDDLY::operation_unchained : public base_unchained {
  public:
    operation_unchained(const ct_initializer::settings &s, operation*);
    virtual ~operation_unchained();

    // required functions

    virtual bool isOperationTable() const   { return true; }
    virtual search_key* initializeSearchKey(operation* op);
    virtual entry_builder& startNewEntry(search_key *key);
    virtual void addEntry();
    virtual void removeStales();
    virtual void removeAll();
  protected:
    virtual void showTitle(output &s) const;
    virtual void showEntry(output &s, int curr) const;
};



// **********************************************************************
// *                                                                    *
// *                    operation_unchained  methods                    *
// *                                                                    *
// **********************************************************************

MEDDLY::operation_unchained
::operation_unchained(const ct_initializer::settings &s, operation* op)
 : base_unchained(s)
{
  global_op = op;
}

MEDDLY::operation_unchained::~operation_unchained()
{
}

MEDDLY::compute_table::search_key* 
MEDDLY::operation_unchained::initializeSearchKey(operation* op)
{
  if (op != global_op)
    throw error(error::UNKNOWN_OPERATION);
  return init(op, 0);
}

MEDDLY::compute_table::entry_builder& 
MEDDLY::operation_unchained::startNewEntry(search_key *key)
{
  MEDDLY_DCASSERT(key);
  startIndexedEntry(smart_cast<old_search_key*>(key), 0, 0);
  return currEntry;
}

void MEDDLY::operation_unchained::addEntry()
{
  addEntryT<0>();
}

void MEDDLY::operation_unchained::removeStales()
{
  removeStalesT<0>();
}
   
void MEDDLY::operation_unchained::removeAll()
{
  for (unsigned i=0; i<tableSize; i++) {
    if (0==table[i]) continue;
    remove<0>(table[i]);
    table[i] = 0;
  }
}

void MEDDLY::operation_unchained::showTitle(output &s) const
{
  s << "Compute table for " << global_op->getName() << " (index " 
    << long(global_op->getIndex()) << ")\n";
}

void MEDDLY::operation_unchained::showEntry(output &s, int curr) const 
{ 
  global_op->showEntry(s, entries + curr);
}


// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                   operation_unchained_fast class                   *
// *                                                                    *
// *                                                                    *
// **********************************************************************

namespace MEDDLY {
  template <int N>
  class operation_unchained_fast : public operation_unchained {
    public:
      operation_unchained_fast(const ct_initializer::settings &s, 
        operation* op) : operation_unchained(s, op) { }
      virtual ~operation_unchained_fast() { }
      virtual search_result& find(search_key *key);
  };
};


// **********************************************************************
// *                                                                    *
// *                  operation_unchained_fast methods                  *
// *                                                                    *
// **********************************************************************


template <int N>
MEDDLY::compute_table::search_result&
MEDDLY::operation_unchained_fast<N>::find(search_key *k)
{
  static old_search_result ANS;
  old_search_key* key = smart_cast <old_search_key*>(k);
  MEDDLY_DCASSERT(key);
  perf.pings++;
  unsigned h = hash(key);
  unsigned hcurr = h;
  int chain;
  ANS.setInvalid();
  for (chain=0; chain<=maxCollisionSearch; chain++, incMod(hcurr) ) {
    int curr = table[hcurr];
    if (0==curr) continue;
    //
    // Check for match
    //
    if (equal_sw(entries+curr, key->rawData(), N)) {
      if (key->getOp()->shouldStaleCacheHitsBeDiscarded()) {
        if (checkStale<0>(hcurr, curr)) {
          // The match is stale.
          // Since there can NEVER be more than one match
          // in the table, we're done!
          break;
        }
      }
      // "Hit"
      perf.hits++;
#ifdef DEBUG_CT
      printf("Found CT entry ");
      global_op->showEntry(stdout, entries + curr);
      // fprintf(stderr, " in slot %u", h);
      printf("\n");
#endif
      ANS.setResult(entries+curr+key->dataLength(), global_op->getAnsLength());
      break;
    };
    //
    // No match; maybe check stale
    //
    if (checkStalesOnFind) {
      checkStale<0>(hcurr, curr);
    }
  } // for chain
  sawSearch(chain);
  return ANS;
}

// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                           base_map class                           *
// *                                                                    *
// *                                                                    *
// **********************************************************************

class MEDDLY::base_map : public base_table {
  protected:
    template <int K>
    class less {
      public:
        inline bool operator() (const int* p, const int* q) const {
          switch (K) {
              case 8:   if (p[7] < q[7]) return true;
                        if (p[7] > q[7]) return false;
              case 7:   if (p[6] < q[6]) return true;
                        if (p[6] > q[6]) return false;
              case 6:   if (p[5] < q[5]) return true;
                        if (p[5] > q[5]) return false;
              case 5:   if (p[4] < q[4]) return true;
                        if (p[4] > q[4]) return false;
              case 4:   if (p[3] < q[3]) return true;
                        if (p[3] > q[3]) return false;
              case 3:   if (p[2] < q[2]) return true;
                        if (p[2] > q[2]) return false;
              case 2:   if (p[1] < q[1]) return true;
                        if (p[1] > q[1]) return false;
              case 1:   return p[0] < q[0];
              case 0:   return false;
              default:  assert(0);
          }
        }
    };
  public:
    base_map(const ct_initializer::settings &s, operation *op);
    virtual ~base_map();
    virtual bool isOperationTable() const   { return true; }
    virtual search_key* initializeSearchKey(operation* op);
    virtual entry_builder& startNewEntry(search_key *key);
  protected:
    inline void showEntry(output &s, int* h) const {
      global_op->showEntry(s, h);
    }
    inline bool isStale(const int* entry) {
      return global_op->isEntryStale(entry);
    }
    inline void removeEntry(int* h) {
      global_op->discardEntry(h);
      delete[] h;
    }
  protected:
    operation* global_op;
    int* current;
};



// **********************************************************************
// *                                                                    *
// *                          base_map methods                          *
// *                                                                    *
// **********************************************************************

MEDDLY::base_map
::base_map(const ct_initializer::settings &s, operation* op)
 : base_table(s)
{
  global_op = op;
  current = 0;
}

MEDDLY::base_map::~base_map()
{
}

MEDDLY::compute_table::search_key* 
MEDDLY::base_map::initializeSearchKey(operation* op)
{
  if (op != global_op)
    throw error(error::UNKNOWN_OPERATION);
  return init(op, 0);
}

MEDDLY::compute_table::entry_builder& 
MEDDLY::base_map::startNewEntry(search_key *key)
{
  MEDDLY_DCASSERT(key);
  if (key->getOp() != global_op)
    throw error(error::UNKNOWN_OPERATION);
  current = startPtrEntry(smart_cast<old_search_key*>(key), 0, 0);
  return currEntry;
}


// **********************************************************************
// *                                                                    *
// *                                                                    *
// *                        operation_map  class                        *
// *                                                                    *
// *                                                                    *
// **********************************************************************

namespace MEDDLY {
  template <int K>
  class operation_map : public base_map {
    public:
      operation_map(const ct_initializer::settings &s, operation* op);
      virtual ~operation_map();

      virtual search_result& find(search_key *key);

      virtual void addEntry();
      virtual void removeStales();
      virtual void removeAll();

      virtual void show(output &s, int verbLevel = 0);
    protected:
      std::map<int*, int*, less<K> > ct;
  };
};

// **********************************************************************
// *                                                                    *
// *                       operation_map  methods                       *
// *                                                                    *
// **********************************************************************

template <int K>
MEDDLY::operation_map<K>
::operation_map(const ct_initializer::settings &s, operation* op)
 : base_map(s, op)
{
}

template <int K>
MEDDLY::operation_map<K>
::~operation_map()
{
}


template <int K>
MEDDLY::compute_table::search_result&
MEDDLY::operation_map<K>::find(search_key *k) 
{
  static old_search_result ANS;
  old_search_key* key = smart_cast <old_search_key*>(k);
  MEDDLY_DCASSERT(key);
  perf.pings++;
  typename std::map <int*, int*, less<K> >::iterator 
    ans = ct.find(key->rawData());
      
  ANS.setInvalid();

  if (ans == ct.end()) {
    return ANS;
  }
  int* h = ans->second;
  if (global_op->shouldStaleCacheHitsBeDiscarded()) {
    if (isStale(h)) {
      ct.erase(ans);
      removeEntry(h);
      return ANS;
    }
  }
#ifdef DEBUG_CT
  printf("Found CT entry ");
  global_op->showEntry(stdout, h);
  printf("\n");
#endif
  perf.hits++;
  ANS.setResult(h+K, global_op->getAnsLength());
  return ANS;
}


template <int K>
void MEDDLY::operation_map<K>::addEntry()
{
#ifdef DEBUG_CT
  printf("Adding CT entry ");
  showEntry(stdout, current);
  printf("\n");
#endif
#ifdef DEVELOPMENT_CODE
  assert(
#endif
  (ct.insert(std::make_pair(current, current)))
#ifdef DEVELOPMENT_CODE
    .second
  )
#endif
  ;
  current = 0;
}

template <int K>
void MEDDLY::operation_map<K>::removeStales()
{
#ifdef DEBUG_SLOW
  fprintf(stdout, "Removing stales in CT (entries %ld)\n", perf.numEntries);
#endif
#ifdef SUMMARY_STALES
  int stales = 0;
#endif

  typename std::map<int*, int*, less<K> >::iterator curr = ct.begin();
  typename std::map<int*, int*, less<K> >::iterator end = ct.end();
  while (curr != end) {
    int* h = curr->second;
    if (isStale(h)) {
      ct.erase(curr++);
      removeEntry(h);
#ifdef SUMMARY_STALES
      stales++;
#endif
    } else {
      ++curr;
    }
  }

#ifdef DEBUG_SLOW
  fprintf(stdout, "Done removing CT stales (entries %ld)\n", perf.numEntries);
#endif
#ifdef SUMMARY_STALES
  printf("CT %s (index %d) removed %d stales\n", 
    global_op->getName(), global_op->getIndex(), stales);
#endif
}

template <int K>
void MEDDLY::operation_map<K>::removeAll()
{
  typename std::map<int*, int*, less<K> >::iterator curr = ct.begin();
  typename std::map<int*, int*, less<K> >::iterator end = ct.end();
  while (curr != end) {
    int* h = curr->second;
    ct.erase(curr++);
    removeEntry(h);
  }
  MEDDLY_DCASSERT(ct.empty());
}

template <int K>
void MEDDLY::operation_map<K>::show(output &s, int verbLevel) 
{
  if (verbLevel < 1) return;
  s << "Compute table for " << global_op->getName() << " (index " 
    << long(global_op->getIndex()) << ")\n";

  s << "\tMap size: " << long(ct.size()) << "\n";
  verbLevel -= 4;
  if (verbLevel<1) return;

  s << "Map entries, in order:\n\t";
  typename std::map<int*, int*, less<K> >::iterator curr = ct.begin();
  typename std::map<int*, int*, less<K> >::iterator end = ct.end();
  const char* comma = "";
  while (curr != end) {
    showEntry(s, curr->second);
    s << comma;
    comma = ", ";
    ++curr;
  }
  s << "\n\n";

  verbLevel--;
  dumpInternal(s, verbLevel);
}




// **********************************************************************
// *                                                                    *
// *                  monolithic_chained_style methods                  *
// *                                                                    *
// **********************************************************************

MEDDLY::monolithic_chained_style::monolithic_chained_style() 
{ 
}

MEDDLY::compute_table* 
MEDDLY::monolithic_chained_style::create(const ct_initializer::settings &s) const 
{
  return new monolithic_chained(s);
}

bool MEDDLY::monolithic_chained_style::usesMonolithic() const 
{
  return true;
}

// **********************************************************************
// *                                                                    *
// *                 monolithic_unchained_style methods                 *
// *                                                                    *
// **********************************************************************


MEDDLY::monolithic_unchained_style::monolithic_unchained_style() 
{ 
}

MEDDLY::compute_table* 
MEDDLY::monolithic_unchained_style::create(const ct_initializer::settings &s) const 
{
  return new monolithic_unchained(s);
}

bool MEDDLY::monolithic_unchained_style::usesMonolithic() const 
{
  return true;
}

// **********************************************************************
// *                                                                    *
// *                  operation_chained_style  methods                  *
// *                                                                    *
// **********************************************************************

MEDDLY::operation_chained_style::operation_chained_style() 
{ 
}

MEDDLY::compute_table* 
MEDDLY::operation_chained_style::create(const ct_initializer::settings &s, operation* op) const 
{
  switch (op->getKeyLength()) {
    case 8:   return new operation_chained_fast<8>(s, op);
    case 7:   return new operation_chained_fast<7>(s, op);
    case 6:   return new operation_chained_fast<6>(s, op);
    case 5:   return new operation_chained_fast<5>(s, op);
    case 4:   return new operation_chained_fast<4>(s, op);
    case 3:   return new operation_chained_fast<3>(s, op);
    case 2:   return new operation_chained_fast<2>(s, op);
    case 1:   return new operation_chained_fast<1>(s, op);
    default:  assert(0);
              return 0;
  }
  assert(0);
  return 0;
}

bool MEDDLY::operation_chained_style::usesMonolithic() const 
{
  return false;
}


// **********************************************************************
// *                                                                    *
// *                 operation_unchained_style  methods                 *
// *                                                                    *
// **********************************************************************


MEDDLY::operation_unchained_style::operation_unchained_style() 
{ 
}

MEDDLY::compute_table* 
MEDDLY::operation_unchained_style::create(const ct_initializer::settings &s, operation* op) const 
{
  switch (op->getKeyLength()) {
    case 8:   return new operation_unchained_fast<8>(s, op);
    case 7:   return new operation_unchained_fast<7>(s, op);
    case 6:   return new operation_unchained_fast<6>(s, op);
    case 5:   return new operation_unchained_fast<5>(s, op);
    case 4:   return new operation_unchained_fast<4>(s, op);
    case 3:   return new operation_unchained_fast<3>(s, op);
    case 2:   return new operation_unchained_fast<2>(s, op);
    case 1:   return new operation_unchained_fast<1>(s, op);
    default:  assert(0);
              return 0;
  }
  assert(0);
  return 0;
}

bool MEDDLY::operation_unchained_style::usesMonolithic() const 
{
  return false;
}


// **********************************************************************
// *                                                                    *
// *                    operation_map_style  methods                    *
// *                                                                    *
// **********************************************************************


MEDDLY::operation_map_style::operation_map_style() 
{ 
}

MEDDLY::compute_table* 
MEDDLY::operation_map_style::create(const ct_initializer::settings &s, operation* op) const 
{
  switch (op->getKeyLength()) {
    case 8:   return new operation_map<8>(s, op);
    case 7:   return new operation_map<7>(s, op);
    case 6:   return new operation_map<6>(s, op);
    case 5:   return new operation_map<5>(s, op);
    case 4:   return new operation_map<4>(s, op);
    case 3:   return new operation_map<3>(s, op);
    case 2:   return new operation_map<2>(s, op);
    case 1:   return new operation_map<1>(s, op);
    default:  assert(0);
              return 0;
  }
  assert(0);
  return 0;
}

bool MEDDLY::operation_map_style::usesMonolithic() const 
{
  return false;
}


