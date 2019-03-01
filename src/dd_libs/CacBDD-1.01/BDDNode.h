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

#ifndef _BDDNODE_
#define _BDDNODE_

#include "Manager.h"

class BDD{
private:
	    friend class XManager;
        friend class XBDDManager;
		DD node;
		XManager *mgr;
public:
		BDD(XManager *m, DD vNode);
		BDD(const BDD &right);
		BDD();
	    ~BDD();

		inline XManager *checkSameManager( const BDD &other) const;
		bool operator==(const BDD& other) const; 
		bool operator!=(const BDD& other) const; 
		BDD& operator=(const BDD& right);  
		BDD  operator!() const;  // Not 
		BDD  operator+(const BDD& other) const; // Or
		BDD  operator*(const BDD& other) const; // And
		BDD  operator^(const BDD& other) const; // Xor 
		BDD  operator<(const BDD& other) const; // Less than
		BDD  operator>(const BDD& other) const; // More than
		BDD  operator<=(const BDD& other) const;// Less equal
		BDD  operator>=(const BDD& other) const;// More equal
		BDD  operator%(const BDD& other) const; // Nor
		BDD  operator|(const BDD& other) const; // Nand
		BDD  operator&(const BDD& other) const; // XNor
        int  Variable();
        BDD  Then() const;
        BDD  Else() const;
        BDD  Exist(const BDD& cube);
        BDD  Universal(const BDD& cube);
        BDD  Restrict(const BDD& other) const;
        BDD  Compose(int v, const BDD& other) const;
        BDD  Permute(const vector<int> &permu) const;
        BDD  AndExist(const BDD& other, const BDD& cube);
        bool IsComp() { return (ISCOMP(node) != 0); }
        void Support(vector<int> &vars);
        		
		XManager *manager() const;
		DD Node() const;
};

class XBDDManager{
private:
    XManager *mgr;

public:
    XBDDManager(int varCount){ mgr = new XManager(varCount);}
    ~XBDDManager(){ delete mgr; }
    void Clear(int varCnt){ mgr->Clear(varCnt); }

    BDD  BddOne(){ return BDD(mgr, mgr->One);}
    BDD  BddZero(){ return BDD(mgr, mgr->Zero);}    
    BDD  BddVar(int varIndex){ return BDD(mgr, varIndex); }  
    BDD  Ite(const BDD &f, const BDD &g, const BDD &h){ return BDD(mgr, mgr->ite(f.node, g.node, h.node)); };   
        
    void   GarbageCollection(){ mgr->GarbageCollection(); };
    void   SetMaxCacheSize(int v){ mgr->SetMaxCacheSize(v); };    
    int    GetUTableCount(){ return mgr->GetUTableCount(); };
    int    GetCacheCount(){ return mgr->GetCacheCount(); };
    int    NodeCount(){ return mgr->NodeCount(); };
    int    DeadCount(){ return mgr->DeadCount(); }
    double Get_CTable_HitRate() { return mgr->Get_CTable_HitRate(); }
    double Get_UTable_HitRate() { return mgr->Get_UTable_HitRate(); }
    XManager *manager() const { return mgr; };

    int    VarDistanceOf(BDD A, BDD B) { return mgr->VarDistanceOf(A.Node(), B.Node());}
    int    LargestPath(BDD from, vector<int> &path){ return mgr->LargestPath(from.Node(), path); }     
    void   ShowInfo(double vtime = 0){ mgr->ShowInfo(vtime); }
    void   ShowInfoToFile(const char *filename){ mgr->ShowInfoToFile(filename); };
    void   ShowInfoToFile(const char *filename, const string &info){ mgr->ShowInfoToFile(filename, info); };
};

#endif
