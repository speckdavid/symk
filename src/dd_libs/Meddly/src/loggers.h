
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


/*! \file loggers.h

    meddly.h should be included before this file.


    Various built-in loggers.
*/


#ifndef LOGGERS_H
#define LOGGERS_H

#include <iostream>

namespace MEDDLY {

  class json_logger;
  class simple_logger;

};


// ******************************************************************
// *                                                                *
// *                        json_logger class                       *
// *                                                                *
// ******************************************************************

/** For logging stats in json format.
*/

class MEDDLY::json_logger : public MEDDLY::forest::logger {
    std::ostream &out;
  public:
    json_logger(std::ostream &s);
    virtual ~json_logger();

    virtual void addComment(const char* comment);
    virtual void newPhase(const forest* f, const char* comment);
    virtual void logForestInfo(const forest* f, const char* name);
    virtual void addToActiveNodeCount(const forest* f, int level, long delta);
};

// ******************************************************************
// *                                                                *
// *                       simple_logger class                      *
// *                                                                *
// ******************************************************************

/** For logging stats in a flexible and compact text-based format.
    No timestamps.
*/

class MEDDLY::simple_logger : public MEDDLY::forest::logger {
    std::ostream &out;
    long** active_delta;
    int* left;
    int* right;
    int batch_forests;
    int aggregate;
    int ucount;
  public:
    simple_logger(std::ostream &s, int agg=16);
    virtual ~simple_logger();

    virtual void addComment(const char* comment);
    virtual void newPhase(const forest* f, const char* comment);
    virtual void logForestInfo(const forest* f, const char* name);
    virtual void addToActiveNodeCount(const forest* f, int level, long delta);

  protected:
    inline long* activeArray(int fid) const {
      return (left[fid]<0) ? active_delta[fid] - left[fid] : active_delta[fid];
    }

    void flushLog();
};

#endif
