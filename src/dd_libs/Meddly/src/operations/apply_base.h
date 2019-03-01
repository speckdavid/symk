
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

#ifndef APPLY_BASE_H
#define APPLY_BASE_H

/*
    Useful base classes for binary apply operations.
*/

namespace MEDDLY {
  class generic_binary_mdd;
  class generic_binary_mxd;
  class generic_binbylevel_mxd;
  class generic_binary_ev;
  class generic_binary_evplus;
  class generic_binary_evtimes;
}

// ******************************************************************

class MEDDLY::generic_binary_mdd : public binary_operation {
  public:
    generic_binary_mdd(const binary_opname* code, expert_forest* arg1, 
      expert_forest* arg2, expert_forest* res);

  protected:
    virtual ~generic_binary_mdd();

  public:
    virtual void discardEntry(const node_handle* entryData);
    virtual void showEntry(output &strm, const node_handle *entryData) const;
    virtual void computeDDEdge(const dd_edge& a, const dd_edge& b, dd_edge &c);

    virtual node_handle compute(node_handle a, node_handle b);

  protected:
    virtual bool isStaleEntry(const node_handle* entryData);

    inline compute_table::search_key* 
    findResult(node_handle a, node_handle b, node_handle &c) 
    {
      compute_table::search_key* CTsrch = useCTkey();
      MEDDLY_DCASSERT(CTsrch);
      CTsrch->reset();
      if (can_commute && a > b) {
        CTsrch->writeNH(b);
        CTsrch->writeNH(a);
      } else {
        CTsrch->writeNH(a);
        CTsrch->writeNH(b);
      }
      compute_table::search_result& cacheFind = CT->find(CTsrch);
      if (!cacheFind) return CTsrch;
      c = resF->linkNode(cacheFind.readNH());
      doneCTkey(CTsrch);
      return 0;
    }

    inline void saveResult(compute_table::search_key* K, 
      node_handle a, node_handle b, node_handle c) 
    {
      arg1F->cacheNode(a);
      arg2F->cacheNode(b);
      compute_table::entry_builder &entry = CT->startNewEntry(K);
      entry.writeResultNH(resF->cacheNode(c));
      CT->addEntry();
    }

  protected:
    // If terminal condition is reached, returns true and the result in c.
    // Must be provided in derived classes.
    virtual bool checkTerminals(node_handle a, node_handle b, node_handle& c) = 0;
    
};

// ******************************************************************

class MEDDLY::generic_binary_mxd : public binary_operation {
  public:
    generic_binary_mxd(const binary_opname* code, expert_forest* arg1, 
      expert_forest* arg2, expert_forest* res);

  protected:
    virtual ~generic_binary_mxd();

  public:
    virtual void discardEntry(const node_handle* entryData);
    virtual void showEntry(output &strm, const node_handle *entryData) const;
    virtual void computeDDEdge(const dd_edge& a, const dd_edge& b, dd_edge &c);

    virtual node_handle compute(node_handle a, node_handle b);

  protected:
    node_handle compute_r(int i, int k, node_handle a, node_handle b);

  protected:
    virtual bool isStaleEntry(const node_handle* entryData);

    inline compute_table::search_key* 
    findResult(node_handle a, node_handle b, node_handle &c) 
    {
      compute_table::search_key* CTsrch = useCTkey();
      MEDDLY_DCASSERT(CTsrch);
      CTsrch->reset();
      if (can_commute && a > b) {
        CTsrch->writeNH(b);
        CTsrch->writeNH(a);
      } else {
        CTsrch->writeNH(a);
        CTsrch->writeNH(b);
      }
      compute_table::search_result &cacheFind = CT->find(CTsrch);
      if (!cacheFind) return CTsrch;
      c = resF->linkNode(cacheFind.readNH());
      doneCTkey(CTsrch);
      return 0;
    }

    inline void saveResult(compute_table::search_key* Key, 
      node_handle a, node_handle b, node_handle c) 
    {
      arg1F->cacheNode(a);
      arg2F->cacheNode(b);
      compute_table::entry_builder &entry = CT->startNewEntry(Key);
      entry.writeResultNH(resF->cacheNode(c));
      CT->addEntry();
    }

  protected:
    // If terminal condition is reached, returns true and the result in c.
    // Must be provided in derived classes.
    virtual bool checkTerminals(node_handle a, node_handle b, node_handle& c) = 0;
};

// ******************************************************************

class MEDDLY::generic_binbylevel_mxd : public binary_operation {
  public:
    generic_binbylevel_mxd(const binary_opname* code, expert_forest* arg1, 
      expert_forest* arg2, expert_forest* res);

  protected:
    virtual ~generic_binbylevel_mxd();

  public:
    virtual void discardEntry(const node_handle* entryData);
    virtual void showEntry(output &strm, const node_handle *entryData) const;
    virtual void computeDDEdge(const dd_edge& a, const dd_edge& b, dd_edge &c);

    virtual node_handle compute(int level, node_handle a, node_handle b);

  protected:
    virtual bool isStaleEntry(const node_handle* entryData);

    inline compute_table::search_key* 
    findResult(int k, node_handle a, node_handle b, node_handle &c) 
    {
      compute_table::search_key* CTsrch = useCTkey();
      MEDDLY_DCASSERT(CTsrch);
      CTsrch->reset();
      CTsrch->write(k);
      if (can_commute && a > b) {
        CTsrch->writeNH(b);
        CTsrch->writeNH(a);
      } else {
        CTsrch->writeNH(a);
        CTsrch->writeNH(b);
      }
      compute_table::search_result &cacheFind = CT->find(CTsrch);
      if (!cacheFind) return CTsrch;
      c = resF->linkNode(cacheFind.readNH());
      doneCTkey(CTsrch);
      return 0;
    }

    inline void saveResult(compute_table::search_key* Key,
      int k, node_handle a, node_handle b, node_handle c) 
    {
      arg1F->cacheNode(a);
      arg2F->cacheNode(b);
      compute_table::entry_builder &entry = CT->startNewEntry(Key);
      entry.writeResultNH(resF->cacheNode(c));
      CT->addEntry();
    }

    node_handle compute_r(int i, int level, node_handle a, node_handle b);

  protected:
    // If terminal condition is reached, returns true and the result in c.
    // Must be provided in derived classes.
    virtual bool checkTerminals(node_handle a, node_handle b, node_handle& c) = 0;
};
    

// ******************************************************************

class MEDDLY::generic_binary_ev : public binary_operation {
  public:
    generic_binary_ev(const binary_opname* code, expert_forest* arg1, 
      expert_forest* arg2, expert_forest* res);

  protected:
    virtual ~generic_binary_ev();

  public:
    virtual void discardEntry(const node_handle* entryData);

  protected:
    virtual bool isStaleEntry(const node_handle* entryData);
};

// ******************************************************************

class MEDDLY::generic_binary_evplus : public generic_binary_ev {
  public:
    generic_binary_evplus(const binary_opname* code, expert_forest* arg1, 
      expert_forest* arg2, expert_forest* res);

  protected:
    virtual ~generic_binary_evplus();

  public:
    virtual void showEntry(output &strm, const node_handle *entryData) const;
    virtual void computeDDEdge(const dd_edge& a, const dd_edge& b, dd_edge &c);

    virtual void compute(int aev, node_handle a, int bev, node_handle b, int& cev, node_handle &c);

  protected:
    inline compute_table::search_key* findResult(int aev, node_handle a, 
      int bev, node_handle b, int& cev, node_handle &c) 
    {
      compute_table::search_key* CTsrch = useCTkey();
      MEDDLY_DCASSERT(CTsrch);
      CTsrch->reset();
      if (can_commute && a > b) {
        CTsrch->write(bev);
        CTsrch->writeNH(b);
        CTsrch->write(aev);
        CTsrch->writeNH(a);
      } else {
        CTsrch->write(aev);
        CTsrch->writeNH(a);
        CTsrch->write(bev);
        CTsrch->writeNH(b);
      }
      compute_table::search_result &cacheFind = CT->find(CTsrch);
      if (!cacheFind) return CTsrch;
      cacheFind.read(cev);
      c = resF->linkNode(cacheFind.readNH());
      doneCTkey(CTsrch);
      return 0;
    }

    inline void saveResult(compute_table::search_key* Key, 
      int aev, node_handle a, int bev, node_handle b, int cev, node_handle c) 
    {
      arg1F->cacheNode(a);
      arg2F->cacheNode(b);
      compute_table::entry_builder &entry = CT->startNewEntry(Key);
      entry.writeResult(cev);
      entry.writeResultNH(resF->cacheNode(c));
      CT->addEntry();
    }

  protected:
    // If terminal condition is reached, returns true and the result in c.
    // Must be provided in derived classes.
    virtual bool checkTerminals(int aev, node_handle a, int bev, node_handle b, 
      int &cev, node_handle &c) = 0;
};

// ******************************************************************

class MEDDLY::generic_binary_evtimes : public generic_binary_ev {
  public:
    generic_binary_evtimes(const binary_opname* code, expert_forest* arg1, 
      expert_forest* arg2, expert_forest* res);

  protected:
    virtual ~generic_binary_evtimes();

  public:
    virtual void showEntry(output &strm, const node_handle *entryData) const;
    virtual void computeDDEdge(const dd_edge& a, const dd_edge& b, dd_edge &c);

    virtual void compute(float aev, node_handle a, float bev, node_handle b, 
      float& cev, node_handle &c);

    virtual void compute_k(int in, int k, float aev, node_handle a,
      float bev, node_handle b, float& cev, node_handle& c);

  protected:
    inline compute_table::search_key* findResult(float aev, node_handle a, 
      float bev, node_handle b, float& cev, node_handle &c) 
    {
      compute_table::search_key* CTsrch = useCTkey();
      MEDDLY_DCASSERT(CTsrch);
      CTsrch->reset();
      if (can_commute && a > b) {
        CTsrch->write(bev);
        CTsrch->writeNH(b);
        CTsrch->write(aev);
        CTsrch->writeNH(a);
      } else {
        CTsrch->write(aev);
        CTsrch->writeNH(a);
        CTsrch->write(bev);
        CTsrch->writeNH(b);
      }
      compute_table::search_result &cacheFind = CT->find(CTsrch);
      if (!cacheFind) return CTsrch;
      cacheFind.read(cev);
      c = resF->linkNode(cacheFind.readNH());
      doneCTkey(CTsrch);
      return 0;
    }

    inline void saveResult(compute_table::search_key* Key, float aev, 
      node_handle a, float bev, node_handle b, float cev, node_handle c) 
    {
      arg1F->cacheNode(a);
      arg2F->cacheNode(b);
      compute_table::entry_builder &entry = CT->startNewEntry(Key);
      entry.writeResult(cev);
      entry.writeResultNH(resF->cacheNode(c));
      CT->addEntry();
    }

  protected:
    // If terminal condition is reached, returns true and the result in c.
    // Must be provided in derived classes.
    virtual bool checkTerminals(float aev, node_handle a,
      float bev, node_handle b, float &cev, node_handle &c) = 0;
};


#endif

