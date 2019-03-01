
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
#include "loggers.h"

#define BATCH

// ******************************************************************
// *                                                                *
// *                       json_logger methods                      *
// *                                                                *
// ******************************************************************

MEDDLY::json_logger::json_logger(std::ostream &s)
  : out(s)
{
}

MEDDLY::json_logger::~json_logger()
{
}

void MEDDLY::json_logger::addComment(const char*)
{
  // Completely ignored
}

void MEDDLY::json_logger::newPhase(const forest*, const char*)
{
  // Completely ignored
}

void MEDDLY::json_logger::logForestInfo(const forest* f, const char* name)
{
  const expert_forest* ef = dynamic_cast<const expert_forest*>(f);
  MEDDLY_DCASSERT(ef);
  if (0==ef) return;

  fixLogger();

  int L = ef->getNumVariables();
  int smallest = ef->isForRelations() ? -L : 1;

  out << "{ \"forest_id\":" << ef->FID() << ", ";
  if (name) {
    out << "\"name\":\"" << name << "\", ";
  }
  out << "\"left\":" << smallest << ", ";
  out << "\"right\":" << L;
  if (recordingNodeCounts()) {
    long* raw_active;
    long* active;
    if (ef->isForRelations()) {
      raw_active = new long[2*L+1];
      active = raw_active+L;
    } else {
      raw_active = new long[L+1];
      active = raw_active;
    }
    ef->countNodesByLevel(active);

    out << ", \"an\":[";
    for (int l=smallest; l<=L; l++) {
      out << active[l];
      if (l<L) out << ", ";
    }
    out << "]";
  }
  out << " }\n";
  out.flush();
}

void MEDDLY::json_logger::addToActiveNodeCount(const forest* f, int level, long delta)
{
  if (0==f) return;
  out << "{ \"f\":" << f->FID() << ", \"l\":" << level << ", \"anc\":" << delta << " }\n";
  out.flush();
}

// ******************************************************************
// *                                                                *
// *                      simple_logger methods                     *
// *                                                                *
// ******************************************************************

MEDDLY::simple_logger::simple_logger(std::ostream &s, int agg)
  : out(s)
{
  out << "T simple\n";
  active_delta = 0;
  left = 0;
  right = 0;
  batch_forests = 0;
  aggregate = MAX(1, agg);
  ucount = aggregate;
}

MEDDLY::simple_logger::~simple_logger()
{
  flushLog();

  for (int i=0; i<batch_forests; i++) {
    delete[] active_delta[i];
  }
  free(active_delta);
  free(left);
  free(right);
}

void MEDDLY::simple_logger::addComment(const char* str)
{
  if (0==str) return;
  out << "# ";
  while (*str) {
    out << *str;
    if ('\n' == *str) out << "# ";
    str++;
  }
  out << "\n";
}

void MEDDLY::simple_logger::newPhase(const forest* f, const char* str)
{
  out << "p ";
  const expert_forest* ef = dynamic_cast<const expert_forest*>(f);
  MEDDLY_DCASSERT(ef);
  // Failsafe
  if (ef) {
    out << ef->FID() << " ";
  } else {
    out << "0 ";
  }

  if (0==str) {
    out << "\n";
    return;
  }
  for (; *str; str++) {
    if ('\n' == *str) continue; // strip any newlines
    out << *str;
  }
  out << "\n";
}

void MEDDLY::simple_logger::logForestInfo(const forest* f, const char* name)
{
  const expert_forest* ef = dynamic_cast<const expert_forest*>(f);
  MEDDLY_DCASSERT(ef);
  if (0==ef) return;

  fixLogger();

  /* Allocate space for this forest info */
  MEDDLY_DCASSERT(batch_forests >= 0);
  if (ef->FID() >= (unsigned int)(batch_forests)) {
    int bf = ((ef->FID() / 16) + 1) * 16;

    /* Increase forest dimension if needed */
    if (recordingNodeCounts()) {
      active_delta = (long**) realloc(active_delta, bf * sizeof(long*));
      if (0==active_delta) throw error(error::INSUFFICIENT_MEMORY);
      for (int i=batch_forests; i<bf; i++) {
        active_delta[i] = 0;
      }
    }

    /* Increase left and right arrays */
    left = (int*) realloc(left, bf * sizeof(int));
    if (0==left) throw error(error::INSUFFICIENT_MEMORY);
    right = (int*) realloc(right, bf * sizeof(int));
    if (0==right) throw error(error::INSUFFICIENT_MEMORY);
    for (int i=batch_forests; i<bf; i++) {
      left[i] = right[i] = 0;
    }

    /* Other data? */

    batch_forests = bf;
  }

  /* Get some forest info */
  int L = ef->getNumVariables();
  right[ef->FID()] = L;
  left[ef->FID()] = ef->isForRelations() ? -L : 1;

  if (recordingNodeCounts()) {
    /* Initialize active_delta array for this forest */
    if (ef->isForRelations()) {
      active_delta[ef->FID()] = new long[2*L+1];
    } else {
      active_delta[ef->FID()] = new long[L+1];
    }
  }

  /* Write */

  out << "F " << ef->FID();
  if (name) {
    out << " \"" << name << "\"";
  }
  out << " " << left[ef->FID()]; 
  out << " " << right[ef->FID()];

  if (recordingNodeCounts()) {
    long* active = activeArray(ef->FID());
    ef->countNodesByLevel(active);

    out << " [";
    for (int l=left[ef->FID()]; l<=right[ef->FID()]; l++) {
      out << active[l];
      active[l] = 0;
      if (l<L) out << ", ";
    }
    out << "]";
  }
  out << "\n";
  out.flush();
}

void MEDDLY::simple_logger
::addToActiveNodeCount(const forest* f, int level, long delta)
{
  if (0==f) return;
#ifdef BATCH
  long* active = activeArray(f->FID());
  active[level] += delta;
  ucount--;
  if (ucount) return;
  ucount = aggregate;
  flushLog();
#else
  out << "a " << f->FID() << " " << level << " " << delta << "\n";
  if (recordingTimeStamps()) {
    long sec, usec;
    currentTime(sec, usec);
    out << "t " << sec << " " << usec << "\n";
  }
  out.flush();
#endif
}

void MEDDLY::simple_logger::flushLog()
{
#ifdef BATCH
  /* Time to write to file */
  if (recordingNodeCounts()) {

    out << "a";

    for (int f=1; f<batch_forests; f++) {
      if (0==active_delta[f]) continue;
      long* active = activeArray(f);
      for (int k=left[f]; k<=right[f]; k++) {
        if (0==active[k]) continue;
        out << " " << f << " " << k << " " << active[k];
        active[k] = 0;
      }
    }

    out << "\n";

    if (recordingTimeStamps()) {
      long sec, usec;
      currentTime(sec, usec);
      out << "t " << sec << " " << usec << "\n";
    }

  }
#endif
  out.flush();
}
