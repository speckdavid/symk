
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

#include "unique_table.h"

#include <limits.h>

MEDDLY::unique_table::unique_table(expert_forest* ef)
: parent(ef)
{
  int num_vars=parent->getNumVariables();
  if(parent->isForRelations()){
    tables = new subtable[2*num_vars+1];
    if(tables==0){
      fprintf(stderr, "Error in allocating array of size %lu at %s, line %d\n",
          (2*num_vars+1)*sizeof(subtable), __FILE__, __LINE__);
      throw error(error::INSUFFICIENT_MEMORY);
    }
    tables += num_vars;

    for(int i=1; i<=num_vars; i++){
      tables[i].init(parent);
      tables[-i].init(parent);
    }
  }
  else{
    tables = new subtable[num_vars+1];
    if(tables==0){
      throw error(error::INSUFFICIENT_MEMORY);
    }

    for(int i=1; i<=num_vars; i++){
      tables[i].init(parent);
    }
  }
}

MEDDLY::unique_table::~unique_table()
{
  if(parent->isForRelations()){
    tables -= parent->getNumVariables();
  }

  delete[] tables;
}

unsigned MEDDLY::unique_table::getSize() const
{
  unsigned num = 0;
  int num_vars = parent->getNumVariables();
  if (parent->isForRelations()) {
    for(int i = 1; i <= num_vars; i++){
      num += tables[i].getSize();
      num += tables[-i].getSize();
    }
  }
  else {
    for (int i = 1; i <= num_vars; i++) {
      num += tables[i].getSize();
    }
  }
  return num;
}

unsigned MEDDLY::unique_table::getNumEntries() const
{
  unsigned num = 0;
  int num_vars = parent->getNumVariables();
  if (parent->isForRelations()) {
    for (int i = 1; i <= num_vars; i++) {
      num += tables[i].getNumEntries();
      num += tables[-i].getNumEntries();
    }
  }
  else {
    for (int i = 1; i <= num_vars; i++) {
      num += tables[i].getNumEntries();
    }
  }
  return num;
}

unsigned MEDDLY::unique_table::getMemUsed() const
{
  unsigned num = 0;
  int num_vars = parent->getNumVariables();
  if (parent->isForRelations()) {
    for (int i = 1; i <= num_vars; i++) {
      num += tables[i].getMemUsed();
      num += tables[-i].getMemUsed();
    }
  }
  else {
    for (int i = 1; i <= num_vars; i++) {
      num += tables[i].getMemUsed();
    }
  }
  return num;
}

void MEDDLY::unique_table::reportStats(output &s, const char* pad, unsigned flags) const
{
  if (flags & expert_forest::UNIQUE_TABLE_STATS) {
    s << pad << "Unique table stats:\n";
    s << pad << "    " << long(getSize()) << " current size\n";
    s << pad << "    " << long(getNumEntries()) << " current entries\n";
  }
}

void MEDDLY::unique_table::show(output &s) const
{
  int num_vars = parent->getNumVariables();
  if (parent->isForRelations()) {
    for (int i = 1; i <= num_vars; i++) {
      s << "Unique table (Var " << i << "):\n";
      tables[i].show(s);
      s << "Unique table (Var " << -i << "):\n";
      tables[-i].show(s);
    }
  }
  else {
    for (int i = 1; i <= num_vars; i++) {
      s << "Unique table (Var " << i << "):\n";
      tables[i].show(s);
    }
  }
  s.flush();
}

MEDDLY::unique_table::subtable::subtable()
: parent(nullptr), table(nullptr)
{
}

MEDDLY::unique_table::subtable::~subtable()
{
  if (parent != nullptr) {
    free(table);
  }
}

void MEDDLY::unique_table::subtable::reportStats(output &s, const char* pad, unsigned flags) const
{
  if (flags & expert_forest::UNIQUE_TABLE_STATS) {
    s << pad << "Unique table stats:\n";
    s << pad << "    " << long(getSize()) << " current size\n";
    s << pad << "    " << long(getNumEntries()) << " current entries\n";
  }
}

void MEDDLY::unique_table::subtable::show(output &s) const
{
  for (unsigned i=0; i < size; i++) {
    if(table[i] != 0) {
      s << "[" << long(i) << "] : ";
      for (int index = table[i]; index; index = parent->getNext(index)) {
        s << index <<" ";
      }
      s.put("\n");
    }
  }
  s.flush();
}

void MEDDLY::unique_table::subtable::init(expert_forest *ef)
{
  parent = ef;
  size = MIN_SIZE;
  num_entries = 0;

  table = static_cast<node_handle*>(calloc(size, sizeof(node_handle)));
  if (table == nullptr) {
    throw error(error::INSUFFICIENT_MEMORY);
  }

  next_expand = 2 * size;
  next_shrink = 0;
}

void MEDDLY::unique_table::subtable::add(unsigned hash, node_handle item)
{
  MEDDLY_DCASSERT(item>0);

  if (num_entries >= next_expand){
    expand();
  }
  num_entries++;

  unsigned h = hash % size;
  parent->setNext(item, table[h]);
  table[h] = item;
}

MEDDLY::node_handle MEDDLY::unique_table::subtable::remove(unsigned hash, node_handle item)
{
  unsigned h = hash%size;

  MEDDLY_CHECK_RANGE(0, h, size);

  node_handle prev = 0;
  for (node_handle ptr = table[h]; ptr!=0; ptr = parent->getNext(ptr)) {
    if (item == ptr) {
      if (ptr != table[h]) {
        // remove from middle
        parent->setNext(prev, parent->getNext(ptr));
      } else {
        // remove from head
        table[h] = parent->getNext(ptr);
      }
      num_entries--;
      if (num_entries < next_shrink) {
        shrink();
      }
      return ptr;
    }
    prev = ptr;
  }
  return 0;
}

void MEDDLY::unique_table::subtable::clear()
{
  if (parent != 0) {
    free(table);
    init(parent);
  }
}

int MEDDLY::unique_table::subtable::getItems(node_handle* items, int sz) const
{
  int k = 0;
  for (unsigned i = 0; i < size; i++) {
    node_handle curr = table[i];
    while (curr != 0) {
      items[k++] = curr;
      if(k == sz){
        return k;
      }
      curr = parent->getNext(curr);
    }
  }

  MEDDLY_DCASSERT(k == num_entries);
  return k;
}

//
// Helpers (private)
//

MEDDLY::node_handle MEDDLY::unique_table::subtable::convertToList()
{
  /*
  printf("Converting to list\n");
  show(stdout);
   */
  node_handle front = 0;
  node_handle next = 0;
  node_handle curr = 0;
  for (unsigned i = 0; i < size; i++) {
    for (curr = table[i]; curr; curr = next) {
      // add to head of list
      next = parent->getNext(curr);
      parent->setNext(curr, front);
      front = curr;
    }
    table[i] = 0;
  } // for i
  num_entries = 0;
  return front;
}

void MEDDLY::unique_table::subtable::buildFromList(node_handle front)
{
  for (node_handle next = 0; front != 0; front = next) {
    next = parent->getNext(front);
    unsigned h = parent->hash(front) % size;
    MEDDLY_DCASSERT(h < size);
    parent->setNext(front, table[h]);
    table[h] = front;
    num_entries++;
  }
}

void MEDDLY::unique_table::subtable::expand()
{
  MEDDLY_DCASSERT(size < MAX_SIZE);
#ifdef DEBUG_SLOW
  fprintf(stderr, "Enlarging unique table (current size: %d)\n", size);
#endif
  node_handle ptr = convertToList();
  // length will the same as num_entries previously
  unsigned newSize = size * 2;
  node_handle* temp = (node_handle*) realloc(table, sizeof(node_handle) * newSize);
  if (0==temp){
    fprintf(stderr, "Error in allocating array of size %lu at %s, line %d\n",
        newSize * sizeof(node_handle), __FILE__, __LINE__);
    throw error(error::INSUFFICIENT_MEMORY);
  }

  table = temp;
  for (unsigned i = newSize-1; i >= size; i--){
    table[i] = 0;
  }

  next_shrink = size;
  size = newSize;
  next_expand = (size >= MAX_SIZE ? UINT_MAX : size * 2);

  buildFromList(ptr);
}

void MEDDLY::unique_table::subtable::shrink()
{
  MEDDLY_DCASSERT(size > MIN_SIZE);
#ifdef DEBUG_SLOW
  fprintf(stderr, "Shrinking unique table (current size: %d)\n", size);
#endif
  node_handle ptr = convertToList();
  // length will the same as num_entries previously
  unsigned newSize = size / 2;
  node_handle *temp = (node_handle*) realloc(table, sizeof(node_handle) * newSize);
  if (0==temp) {
    fprintf(stderr, "Error in allocating array of size %lu at %s, line %d\n",
        newSize * sizeof(node_handle), __FILE__, __LINE__);
    throw error(error::INSUFFICIENT_MEMORY);
  }
  table = temp;
  next_expand = size;
  size = newSize;
  if (size <= MIN_SIZE) next_shrink = 0;
  else                 next_shrink = size / 2;
  buildFromList(ptr);
}
