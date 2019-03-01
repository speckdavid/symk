/*****************************************************************************
Copyright (c) 2012 - 2013, The Board of Trustees of the University of Beijing Technology and
the University of Griffith.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the
  above copyright notice, this list of conditions
  and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the University of Beijing Technology and
  the University of Griffith nor the names of its contributors 
  may be used to endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

/*****************************************************************************
written by
   Guanfeng Lv, last updated 10/26/2012
*****************************************************************************/

#ifndef _MANAGER_
#define _MANAGER_

#include "UTable.h"
#include "CTable.h"
#include "XVector.h"
#include "XBits.h"
#include "XInts.h"
#include <limits.h>
#include <vector>
#include <set>
//#include <sys/sysinfo.h>
using namespace std;
//===================================================
class XManager{
private:	
    friend class XCTable;
    friend class XUTable;	
    int varCount; 
    int maxCacheSize;
    DdNodes  nodes;
    
    double    oldCHit;
    long long oldCallCount;
    long long curCallCount;

	
    XUTable *UTable;
    XCTable *CTable;		
        
    int  GetFreeNode(){ return nodes.GetFreeNode(); };
    DD   PermuteRecur(DD A, int *permu, XInts *visited);
    DD   ExistRecur(DD f, DD cube);
    DD   AndRecur(DD f, DD g);
    DD   XorRecur(DD f, DD g);
    DD   IteRecur(DD f, DD g, DD h);
    
    int  adjustCTableType;
    void AdjustCTable();
    unsigned long GetAvailMem();
    
public:
    set<void *> externalBDDs;
    
public:	    
    DD  One;
    DD  Zero;
    DD  ite(DD F, DD G, DD H);
    inline int Variable(DD A);
    inline int Top_Variable(DD A, DD B);
    inline int Top_Variable(DD A, DD B, DD C);
    inline DD  Then(DD A, int v);
    inline DD  Else(DD A, int v); 
    inline DD  Not(DD A) { return A ^ 0x80000000; }
    inline DD  Or(DD A, DD B) { return Not(And(Not(A), Not(B)));} 
    inline DD  Nor(DD A, DD B){ return And(Not(A), Not(B));}  
    inline DD  And(DD A, DD B){ return AndRecur(A, B);} 
    inline DD  Nand(DD A, DD B){ return Not(AndRecur(A, B));} 
    inline DD  Xor(DD A, DD B) { return XorRecur(A, B);}
    inline DD  Xnor(DD A, DD B){ return XorRecur(A, Not(B));}
    inline DD  LessThan(DD A, DD B){ return ite(A,Zero,B); }
    inline DD  MoreThan(DD A, DD B){ return ite(A,Not(B),Zero); }
    inline DD  LessEqual(DD A, DD B){ return ite(A,B,One); }
    inline DD  MoreEqual(DD A, DD B){ return ite(A,One,Not(B)); };    
    inline DD  IsConstant(DD A) { return (ADDR(A)==0);};
    DD   And(const XVector<DD> dds);
    DD   Or(const XVector<DD> dds);    
    DD   Exist(DD A, DD cube);
    DD   Universal(DD A, DD cube);    
    DD   Restrict(DD F, DD C);
    DD   Compose(DD f, int v, DD g); 
    DD   Permute(DD A, int *permu);  
    DD   AndExist(DD f, DD g, DD cube);
    void Support(DD node, vector<int> &vars);

    int  VarDistanceOf(DD A, DD B);  
    int  LargestPath(DD from, vector<int> &path); 
    int  NodeCount() {return nodes.NodeCount(); };
    int  DeadCount();
    void GarbageCollection();
    
public:
    XManager(int variableCount);
    ~XManager();
    void   Clear(int varCnt);
    void   ShowInfo(double vtime);
    void   ShowInfoToFile(const char *filename);
    void   ShowInfoToFile(const char *filename, const string &info);
    void   SetMaxCacheSize(int v){ maxCacheSize = v; };
    int    GetVariableCount();
    int    GetNodeCount();
    int    GetUTableCount() { return UTable->count; };
    int    GetCacheCount() { return CTable->Count(); };
    double Get_CTable_HitRate(){ return CTable->HitRate();};	
    double Get_UTable_HitRate(){ return UTable->HitRate();};
    void   PrintNode(DdNode *node);
};
//======================================================
inline int XManager::Variable(DD A)
{
    DdNode *n = &nodes[ADDR(A)];
    return n->var;
}

inline int XManager::Top_Variable(DD A, DD B)
{
    DdNode *NA = &nodes[ADDR(A)];
    DdNode *NB = &nodes[ADDR(B)];
    return ((NA->var <= NB->var)?NA->var:NB->var);
}

inline int XManager::Top_Variable(DD A, DD B, DD C)
{
    DdNode *NA = &nodes[ADDR(A)];
    DdNode *NB = &nodes[ADDR(B)];
    DdNode *NC = &nodes[ADDR(C)];

    if (NA->var <= NB->var){
        if (NA->var <= NC->var) return NA->var;
        return NC->var;
    }
    else{
        if (NB->var <= NC->var) return NB->var;
        return NC->var; 
    }
};

inline DD XManager::Then(DD A, int v)
{
    DdNode *n = &nodes[ADDR(A)];
    if (n->var == v) {
        if (ISCOMP(A)) return Not(n->Then);
        else return n->Then;
    }
    return A;
};

inline DD XManager::Else(DD A, int v) 
{
    DdNode *n = &nodes[ADDR(A)];
    if (n->var == v) {
        if (ISCOMP(A)) return Not(n->Else);
        else return n->Else;
    }
    return A;
};

inline void XManager::AdjustCTable()
{   
      curCallCount++;        
      if(curCallCount >= oldCallCount){        
          double curCHit = CTable->HitRate();           
          if(curCHit >= oldCHit || CTable->count < NodeCount() * CTable->HitRate()){
              int addCount = 1;
              CTable->size_inc(addCount);            
          }
          oldCHit = curCHit;         
          oldCallCount = (curCallCount<<1);
          if(GetAvailMem() < (ONE_MILLION << 8) ) GarbageCollection();
          if(GetAvailMem() < (ONE_MILLION << 8) ) CTable->Expand(false);          
      } 
};

inline unsigned long XManager::GetAvailMem()
{
    return 1000000000;
    //struct sysinfo info;
    //sysinfo(&info);
    //return info.freeram;
}

inline int XManager::VarDistanceOf(DD A, DD B)
{
    int av = nodes[ADDR(A)].var;
    int bv = nodes[ADDR(B)].var;
    if(av == INT_MAX){
        if(bv == INT_MAX) return 0;
        else return (varCount - bv + 1);
    }
    else{
        if(bv == INT_MAX){
            return (varCount - av + 1);
        }
        else{
            int t = av - bv;
            return ((t > 0)?t:(-t));
        }
    }
};

#endif
