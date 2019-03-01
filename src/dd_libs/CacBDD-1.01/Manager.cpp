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
   Guanfeng Lv and Kaile Su, last updated 14/07/2015
*****************************************************************************/

#include "Base.h"
#include "Manager.h"
#include "Timer.h"
#include "XBits.h"
#include "XFunction.h"
#include "BDDNode.h"

#include <cstdio>
#include <assert.h>
#include <cmath>
#include <limits.h>
#include <iostream>
#include <algorithm> 
#include <fstream>
using namespace std;

XManager::XManager(int variableCount)
{
    varCount  = variableCount;    
    maxCacheSize = 2147483647; 
    nodes.Init(1 * ONE_MILLION);

    UTable = new XUTable(this, 18); 
    CTable = new XCTable(this, 18); 
    
    One = 0;
    Zero = Not(One);
    nodes[0].var = INT_MAX;

 
    for(int i=1; i <= varCount; i++){
        UTable->Find_or_Add_Unique_Table(i, One, Zero);
    }
    
    oldCHit = 0.0;
    oldCallCount = 2000000;
    curCallCount = 0;
    adjustCTableType = CACHE_DYN;
}

XManager::~XManager()
{
    delete UTable;
    delete CTable;
}

void XManager::Clear(int varCnt)
{
    varCount  = varCnt;
    nodes.Clear();
    nodes.Init(1 * ONE_MILLION);
    UTable->Clear();
    CTable->Clear();
    nodes[0].var = INT_MAX;    
    for(int i=1; i <= varCount; i++){         
        UTable->Find_or_Add_Unique_Table(i, One, Zero);
    }
}

DD  XManager::AndRecur(DD f, DD g)
{    
    AdjustCTable();
    DD F = ADDR(f);
    DD G = ADDR(g);
        
    if(F == G){
        if(f == g) return (f);
        else return Zero;
    }    
    if(F == One){
        if( f == One) return g;
        else return f;
    }
    if(G == One){
        if(g ==One) return f;
        else return g;
    }
    
    if(F > G){
        DD tmp = f;
        f = g;
        g = tmp;
        F = ADDR(f);
        G = ADDR(g);
    }
    
    DD r;
    bool b = CTable->Find(AND, f, g, r);
    if(b){  return r;}

    int topf = nodes[F].var;
    int topg = nodes[G].var;
    int index;
    DD  fv, fnv, gv, gnv;

    if(topf <= topg){
        index = topf;
        fv  = nodes[F].Then;
        fnv = nodes[F].Else;        

        if(ISCOMP(f)){      
            fv  = Not(fv);
            fnv = Not(fnv);            
        }
    }
    else{
        index = topg;
        fv = fnv = f;
    }

    if(topg <= topf){
        gv  = nodes[G].Then;
        gnv = nodes[G].Else;        
        if(ISCOMP(g)){            
            gv = Not(gv);
            gnv= Not(gnv);
        }
    }
    else{        
        gv = gnv = g;
    }
    
    DD t = AndRecur(fv, gv);
    DD e = AndRecur(fnv, gnv);    
    
    if(t == e){ 
        r = t; 
    }
    else{
        if(ISCOMP(t)){
            r = UTable->Find_or_Add_Unique_Table(index, Not(t), Not(e));
            r = Not(r);
        }
        else{
            r = UTable->Find_or_Add_Unique_Table(index, t, e);
        }        
    }
    
    CTable->Insert(AND,f, g, r);
    return r;
}

DD  XManager::XorRecur(DD f, DD g)
{
    AdjustCTable();
    if(f == g) return Zero;
    if(f == Not(g)) return One;
    if(ADDR(f) > ADDR(g)){
        DD tmp = f;
        f = g;
        g = tmp;
    }
    if(g == Zero) return f;
    if(g == One) return Not(f);
    if(ISCOMP(f)){
        f = Not(f);
        g = Not(g);
    }
    
    if(f == One) return Not(g);

    
    DD r;
    bool b = CTable->Find(XOR,f,g,r);
    if(b) { return r; }

    int topf = nodes[ADDR(f)].var;
    int topg = nodes[ADDR(g)].var;
    int index;
    DD  fv, fnv, gv, gnv;
    if(topf <= topg){
        index = topf;
        fv = nodes[ADDR(f)].Then;
        fnv= nodes[ADDR(f)].Else;
    }
    else{
        index = topg;
        fv = fnv = f;
    }

    if(topg <= topf){
        gv = nodes[ADDR(g)].Then;
        gnv= nodes[ADDR(g)].Else;
        if(ISCOMP(g)){
            gv = Not(gv);
            gnv= Not(gnv);
        }
    }
    else{
        gv = gnv = g;
    }

    DD t = XorRecur(fv, gv);
    DD e = XorRecur(fnv, gnv);
    if(t == e) { r = t; }
    else{
        if(ISCOMP(t)){
            r = UTable->Find_or_Add_Unique_Table(index, Not(t), Not(e));
            r = Not(r);
        }
        else{
            r = UTable->Find_or_Add_Unique_Table(index, t, e);
        }
    }
    CTable->Insert(XOR,f,g,r);
    return r;
}

DD  XManager::IteRecur(DD f, DD g, DD h)
{       
    if(f == One) return g;
    if(f == Zero) return h;

    
    DD r;
    if(g == One || f == g){ 
        if(h == Zero){
            return f;
        }
        else{
            r = AndRecur(Not(f), Not(h));
            return Not(r);
        }
    }
    else if(g == Zero || f == Not(g)){ 
        if( h == One){
            return Not(f);
        }
        else{
            r = AndRecur(Not(f), h);
            return r;
        }
    }

    
    if(g == h){ 
        return g;
    }
    else if(g == Not(h)){
        r = XorRecur(f, h);
        return r;
    }
    
    AdjustCTable();
    
    if (ISCOMP(f)) { 
	    f = Not(f);
	    r = g;
	    g = h;
	    h = r;
    }
    int comple = 0;
    if (ISCOMP(g)) { 
	    g = Not(g);
	    h = Not(h);
	    comple = 1;
    }

    int topf = nodes[ADDR(f)].var;
    int topg = nodes[ADDR(g)].var;
    int toph = nodes[ADDR(h)].var;
    int v = Top_Variable(g,h);
    
    if(topf < v && nodes[ADDR(f)].Then == One && nodes[ADDR(f)].Else == Zero){
        r = UTable->Find_or_Add_Unique_Table(topf, g, h);
        return (comple)?Not(r):r;
    }
    
    bool b = CTable->Find(f,g,h,r);
    if(b){ return (comple)?Not(r):r; } 

    
    int index;
    DD fv, fnv, gv, gnv, hv, hnv;
    if(topf <= v){
        v = Min(topf, v);
        index = topf;
        fv = nodes[ADDR(f)].Then;
        fnv= nodes[ADDR(f)].Else;
    }
    else{ fv = fnv = f;}

    if(topg == v){
        index = topg;
        gv = nodes[ADDR(g)].Then;
        gnv= nodes[ADDR(g)].Else;
    }
    else{ gv = gnv = g;}

    if(toph == v){
        index = toph;
        hv = nodes[ADDR(h)].Then;
        hnv= nodes[ADDR(h)].Else;
        if(ISCOMP(h)){
            hv = Not(hv);
            hnv= Not(hnv);
        }
    }
    else { hv = hnv = h; }

    DD t = IteRecur(fv, gv, hv);
    DD e = IteRecur(fnv, gnv, hnv);
    r = UTable->Find_or_Add_Unique_Table(index,t,e);
    r = (t == e)?t:r;
    CTable->Insert(f,g,h,r);
    return (comple)?Not(r):r;
}

DD XManager::ite(DD F, DD G, DD H)
{ 
    return IteRecur(F, G, H);
}

DD  XManager::And(const XVector<DD> dds)
{
    if(dds.size() < 2) return Zero;

    DD r = And(dds[0], dds[1]);
    for(int i=2; i<dds.size(); i++){
        r = And(r, dds[i]);
    }
    return r;
}

DD  XManager::Or(const XVector<DD> dds)
{
    if(dds.size() < 2) return Zero;

    DD r = Or(dds[0], dds[1]);
    for(int i=2; i<dds.size(); i++){
        r = Or(r, dds[i]);
    }
    return r;
}

DD XManager::PermuteRecur(DD A, int *permu, XInts *visited)
{  
    if(A == One || A == Zero) return A;
    
    int isComple = ISCOMP(A);
    DD r = visited->get_value(ADDR(A));
    if(r != 0){ return (isComple)?Not(r):r; }

    DdNode *node = &nodes[ADDR(A)];
    DD T = PermuteRecur(node->Then, permu, visited);
    DD E = PermuteRecur(node->Else, permu, visited);
        
    r = ite(permu[node->var], T, E);
        
    visited->set_value(ADDR(A), r); 
    r = (isComple)?Not(r):r;    
    return r;
} 

DD  XManager::Permute(DD A, int *permu)
{        
    XInts *visited = new XInts;
    visited->set_size(NodeCount());    
    DD r = PermuteRecur(A, permu, visited);
    delete visited; 
    return r;
}

DD  XManager::ExistRecur(DD f, DD cube)
{      
    if(ADDR(f) == 0 || cube == One){        
        return f;
    }
    DdNode *fnode = &nodes[ADDR(f)];
    DdNode *cnode = &nodes[ADDR(cube)];

    while (fnode->var > cnode->var) {        
        cube = cnode->Then;
        cnode = &nodes[ADDR(cube)];        
        if (cube == One) return(f);
    }
    
    DD r;
    bool b = CTable->Find(EXIST, f, cube, r);
    if(b){
        return r;
    }

    DD T = fnode->Then; 
    DD E = fnode->Else;
    if (ISCOMP(f)){
        T = Not(T); 
        E = Not(E);
    }

    if (fnode->var == cnode->var) {
        if (T == One || E == One || T == Not(E)) {
            return(One);
        }
        DD res1 = ExistRecur(T, cnode->Then);
        if (res1 == One) {
            CTable->Insert(EXIST, f, cube, One);
            return(One);
        }
        DD res2 = ExistRecur(E, cnode->Then);
        DD res = Or(res1, res2);
        CTable->Insert(EXIST, f, cube, res);
        return(res);
    }
    else { 
        DD res1 = ExistRecur(T, cube);
        DD res2 = ExistRecur(E, cube);
        DD res = ite(fnode->var, res1, res2);
        CTable->Insert(EXIST, f, cube, res);
        return(res);
    }
}

DD XManager::Exist(DD A, DD cube)
{	
    return ExistRecur(A, cube);    
}

DD XManager::Universal(DD A, DD cube)
{
    DD X = Not(A);
    return Not(Exist(X, cube));
}

DD XManager::AndExist(DD f, DD g, DD cube)
{
    AdjustCTable();
    
    if(f == Zero || g == Zero || f == Not(g)) return Zero;
    if(f == One && g == One) return One;

    if(cube == One) return (AndRecur(f, g));
    if(f == One || f == g) return (Exist(g, cube));
    if(g == One) return (Exist(f, cube));

    
    if(ADDR(f) > ADDR(g)){
        DD tmp = f;
        f = g;
        g = tmp;
    } 

    DD F = ADDR(f);
    DD G = ADDR(g);
    int topf = nodes[ADDR(f)].var;
    int topg = nodes[ADDR(g)].var;
    int top  = Min(topf, topg);
    int topcube = nodes[ADDR(cube)].var;
    while(topcube < top){
        cube = nodes[ADDR(cube)].Then;
        if(cube == One) return AndRecur(f, g);
        topcube = nodes[ADDR(cube)].var;
    }

    if(ISCOMP(cube) == 0){
        cube = Not(cube);
    }
    
    DD r;
    if(ISCOMP(f)){
        if(F < OP_MAX_VALUE){            
            r = AndRecur(f, g);
            r = Exist(r, cube);
            return r;
        }
    } 
    
    
    bool b = CTable->Find(f, cube, g, r);
    if(b){
        return r;
    }
    
    DD ft, fe, gt, ge;
    int index;
    if(topf == top){
        index = nodes[F].var;
        ft = nodes[F].Then;
        fe = nodes[F].Else;
        if(ISCOMP(f)){
            ft = Not(ft);
            fe = Not(fe);
        }
    }
    else{
        index = nodes[G].var;
        ft = fe = f;
    }

    if(topg == top){
        gt = nodes[G].Then;
        ge = nodes[G].Else;
        if(ISCOMP(g)){
            gt = Not(gt);
            ge = Not(ge);
        }
    }
    else{
        gt = ge = g;
    }
    
    DD t, e;
    if(topcube == top){
        DD ct = nodes[ADDR(cube)].Then;
        t = AndExist(ft, gt, ct);
        if(t == One || t == fe || t == ge){
            CTable->Insert(f, cube, g, t);
            return t;
        }        
        if(t == Not(fe)){
            e = Exist(ge, ct);
        }
        else if(t == Not(ge)){
            e = Exist(fe, ct);
        }
        else{
            e = AndExist(fe, ge, ct);
        }
        if(t == e){
            r = t;
        }
        else{
            r = AndRecur(Not(t), Not(e));
            r = Not(r);
        }
    }
    else{
        t = AndExist(ft, gt, cube);
        e = AndExist(fe, ge, cube);
        if(t == e){
            r = t;
        }
        else{
            if(ISCOMP(t)){
                r = UTable->Find_or_Add_Unique_Table(index, Not(t), Not(e));
                r = Not(r);
            }
            else{
                r = UTable->Find_or_Add_Unique_Table(index, t, e);
            }
        }
    }
    CTable->Insert(f, cube, g, r);
    return r;
}

void XManager::Support(DD dd, vector<int> &vars)
{
    int *index = new int[varCount+1];
    for(int i=0; i<varCount+1; i++){
        index[i] = 0;
    }
    //----------------------------------
    XBits bits;
    bits.set_size(NodeCount());

    XVector<DD> one;
    XVector<DD> two;
    one.push_back(dd);
    XVector<DD> *cur = &one;
    XVector<DD> *nex = &two;
    while(cur->size() > 0){
        for(int i=0; i<cur->size(); i++){
            int address = ADDR((*cur)[i]);
            if(address != 0 && bits.get_value(address) == 0){
                bits.set_value(address, 1);
                DdNode &node = nodes[address];
                index[node.var] = 1;
                nex->push_back(node.Then);
                nex->push_back(node.Else);
            }
        }
        XVector<DD> *tmp = cur;
        cur = nex;
        nex = tmp;
        nex->clear();
    }
    //----------------------------------
    for(int i=1; i<varCount+1; i++){
        if(index[i] == 1) vars.push_back(i);
    }    
    delete []index;
}

DD XManager::Restrict(DD F, DD C)
{
    if (C == One) return F;
    if (C == Zero) return Zero;
    if ((F == One)||(F == Zero)) return F;
    if (F == C) return One;
    if (F == !C) return Zero;

    DD r;
    bool b = CTable->Find(RESTRICT, F, C, r);
    if(b){
        return r;
    }

    int topf = nodes[ADDR(F)].var;
    int topc = nodes[ADDR(C)].var;
    if(topc < topf){ 
        DD H  = nodes[ADDR(C)].Then; 
        DD L  = nodes[ADDR(C)].Else; 
        if(ISCOMP(C)){
            H = Not(H);
            L = Not(L);
        }
        DD r = Restrict(F, Or(H, L));
        CTable->Insert(RESTRICT, F, C, r);
        return r;
    }
    else if(topf < topc){ 
        DD f1 = nodes[ADDR(F)].Then;
        DD f0 = nodes[ADDR(F)].Else;
        if(ISCOMP(F)){
            f1 = Not(f1);
            f0 = Not(f0);
        }
        DD a1 = Restrict(f1, C);
        DD a0 = Restrict(f0, C);
        DD r = ite(topf, a1, a0);        
        CTable->Insert(RESTRICT, F, C, r);
        return r;
    }
    else{ 
        DD f1 = nodes[ADDR(F)].Then;
        DD f0 = nodes[ADDR(F)].Else;
        if(ISCOMP(F)){
            f1 = Not(f1);
            f0 = Not(f0);
        }
        DD H = nodes[ADDR(C)].Then; 
        DD L = nodes[ADDR(C)].Else; 
        if(ISCOMP(C)){
            H = Not(H);
            L = Not(L);
        }
        DD r;
        if(L == Zero){
            r = Restrict(f1, H);
        }
        else if(H == Zero){
            r = Restrict(f0, L);
        }
        else {
            r = ite(topf, Restrict(f1, H), Restrict(f0, L));            
        }
        CTable->Insert(RESTRICT, F, C, r);
        return r;
    }
}

DD XManager::Compose(DD f, int v, DD g)
{
    int topf = nodes[ADDR(f)].var;
    if(topf > v){
        return f;
    }

    bool comple = true;
    DD F = f;
    if(!ISCOMP(f)){
        comple = false;
        F = Not(f);
    }    
    DD r;
    bool b = CTable->Find(v, F, g, r); 
    if(b){
        return (comple?r:Not(r));
    }

    DD f1, f0, g1, g0;
    if(topf == v){
        f1 = Not(nodes[ADDR(F)].Then);
        f0 = Not(nodes[ADDR(F)].Else);
        r = ite(g, f1, f0);        
    }
    else{
        int topg = nodes[ADDR(g)].var;        
        int top;
        if(topf > topg){
            top = topg;
            f1 = f0 = F;
        }
        else{
            top = topf;
            f1 = Not(nodes[ADDR(F)].Then);
            f0 = Not(nodes[ADDR(F)].Else);
        }

        if(topg > topf){
            g1 = g0 = g;
        }
        else{
            g1 = nodes[ADDR(g)].Then;
            g0 = nodes[ADDR(g)].Else;
            if(ISCOMP(g)){
                g1 = Not(g1);
                g0 = Not(g0);
            }
        }
        DD t = Compose(f1, v, g1);
        DD e = Compose(f0, v, g0);        
        r = ite(top, t, e);
    }
    CTable->Insert(v, F, g, r);
    return (comple?r:Not(r));
}

//=======================================================

int  XManager::LargestPath(DD from, vector<int> &path) 
{
    double startTime = Timer::GetTime();

    class XPathNode{
    public:
        int priorId;
        int length;
        XPathNode():priorId(0),length(0){};
    };
        
    path.clear();
    if(ADDR(from) >=NodeCount()  || ADDR(from) <= 0){  
        return 0;
    }
    XPathNode *ps = new XPathNode[NodeCount()];    
    char *inIds = new char[NodeCount()];
        
    XVector<int> levelOne;
    XVector<int> levelTwo;    
    XVector<int> *curLevel = &levelOne;
    XVector<int> *nextLevel= &levelTwo;
    curLevel->push_back(from);
    
    for(int ki=nodes[ADDR(from)].var; ki<=varCount; ki++){        
        if(curLevel->size() == 0) break;        
        memset(inIds, 0, NodeCount());
        for(int i=0; i<curLevel->size(); i++){
            if(!IsConstant((*curLevel)[i])){
                int cId = ADDR((*curLevel)[i]);
                DdNode *curNode = &nodes[cId];
                DD T = curNode->Then;
                DD E = curNode->Else;
                if(ISCOMP((*curLevel)[i])){
                    T = Not(T);
                    E = Not(E);
                }
                if(!IsConstant(T) && inIds[ADDR(T)] == 0){
                    nextLevel->push_back(T);
                    inIds[ADDR(T)] = 1;
                }
                if(!IsConstant(E) && T != E && inIds[ADDR(E)] == 0){
                    nextLevel->push_back(E);
                    inIds[ADDR(E)] = 1;
                }

                if(T != Zero){
                    int varBetween = this->VarDistanceOf(cId, T)-1;                    
                    if(ps[ADDR(T)].length < ps[cId].length + varBetween + 1){                        
                        ps[ADDR(T)].priorId = (*curLevel)[i]; //cId;
                        ps[ADDR(T)].length = ps[cId].length + varBetween + 1;
                    }
                }
                if(E != Zero){                    
                    int varBetween = this->VarDistanceOf(cId, E)-1;
                    if(ps[ADDR(E)].length < ps[cId].length + varBetween){
                        ps[ADDR(E)].priorId = (*curLevel)[i]; //cId;
                        ps[ADDR(E)].length = ps[cId].length + varBetween;
                    }
                }
            }            
        }  

        if(curLevel == &levelOne){            
            levelOne.clear();
            curLevel = &levelTwo;
            nextLevel= &levelOne;            
        }
        else if(curLevel == &levelTwo){            
            levelTwo.clear();        
            curLevel = &levelOne;
            nextLevel= &levelTwo;
        }        
    }
        
    int i = 0;
    while(ps[i].priorId != 0){
        int pId = ADDR(ps[i].priorId);
        if(ADDR(nodes[pId].Then) == i){
            int fvar = nodes[pId].var;
            int tvar = (i==0)?(varCount+1):nodes[i].var;
            if(i == 0 && ISCOMP(ps[i].priorId)){
                fvar++;
            }
            for(int k=fvar; k<tvar; k++){
                path.push_back(k);
            }
        }
        else if(ADDR(nodes[pId].Else) == i){
            int fvar = nodes[pId].var;
            int tvar = (i==0)?(varCount+1):nodes[i].var;
            for(int k=fvar+1; k<tvar; k++){
                path.push_back(k);
            }
        }        
        i = pId;
    }
    for(int k=1; k<nodes[ADDR(from)].var; k++){
        path.push_back(k);
    }    
    delete []ps;
    delete []inIds;
    
    return (int)path.size();
}

int XManager::DeadCount()
{    
    char *c = new char[NodeCount()];
    memset(c, 0, NodeCount());

    set<void *>::iterator it = externalBDDs.begin();
    for(; it != externalBDDs.end(); it++){
        BDD *b = (BDD *)*it;
        c[ADDR(b->node)] = 1;        
    }
    for(int i=NodeCount()-1; i>=1; i--){                
        if(c[i]){
            DdNode *n = &nodes[i];
            c[ADDR(n->Then)] = 1;
            c[ADDR(n->Else)] = 1;
        }
    }
    
    int zeroCnt = 0;    
    for(int i = varCount+1; i<NodeCount(); i++){                        
        if(c[i] == 0){
            zeroCnt++;
        }            
    }
    
    delete []c;
    return zeroCnt;
}

void XManager::GarbageCollection()
{
    char *c = new char[NodeCount()];
    memset(c, 0, NodeCount());
    for(int i=0; i<=varCount; i++) c[i] = 1;
    vector<DD> exDDs;
    
    set<void *>::iterator it = externalBDDs.begin();
    for(; it != externalBDDs.end(); it++){
        BDD *b = (BDD *)*it;
        c[ADDR(b->node)] = 1;        
        exDDs.push_back(ADDR(b->node));        
    }

    for(int i=NodeCount()-1; i>=1; i--){                
        if(c[i]){
            DdNode *n = &nodes[i];
            c[ADDR(n->Then)] = 1;
            c[ADDR(n->Else)] = 1;
        }
    }
    
    DD *ds = new DD[NodeCount()];
    for(int i=0; i<NodeCount(); i++){
        ds[i] = i;
    }
    int curDead = varCount+1;
    int curLive = varCount+1;
    int newcount;
    
    for(int i = varCount+1; i<NodeCount(); i++){                        
        if(c[i] == 1){
            newcount = i;
            for(int k=curDead; k<i; k++){
                if(c[k] == 0){
                    ds[i] = k;
                    nodes[k] = nodes[i];
                    c[i] = 0;
                    c[k] = 1;
                    curDead = k+1;
                    newcount = k+1;
                    break;
                }
            }
        }
    }
    
    for(int i = varCount+1; i<NodeCount(); i++){
        nodes[i].Then = ds[nodes[i].Then];

        bool iscomp = false;
        if(ISCOMP(nodes[i].Else)) iscomp = true;
        
        nodes[i].Else = ds[ADDR(nodes[i].Else)];
        if(iscomp){
            nodes[i].Else = Not(nodes[i].Else);
        }        
    }   
    
    it = externalBDDs.begin();
    for(; it != externalBDDs.end(); it++){
        BDD *b = (BDD *)*it;

        if(b->IsComp()){            
            b->node = Not(ds[ADDR(b->node)]);
        }
        else b->node = ds[b->node];
    }
    
    nodes.nodeCount = newcount;

    UTable->Refresh();
    CTable->Refresh();
    
    delete []ds;
    delete []c;
}

int XManager::GetVariableCount()
{
    return varCount;
}

int XManager::GetNodeCount()
{
    return NodeCount();
}

void XManager::PrintNode(DdNode *node)
{
    cout<<node->var<<", Then:"<<node->Then<<",Else:"<<ADDR(node->Else)<<",Next:"<<node->Next<<endl;
}

void  XManager::ShowInfo(double vtime)
{   
    double mem = (20.0*(NodeCount() + GetCacheCount()) + 4.0*GetUTableCount()) / ONE_MILLION;    
    cout<<"NC:"<<GetNodeCount()<<", CC:"<<GetCacheCount()<<", Mem:"<<mem<<", Hit:"<<CTable->HitRate()<<", Time:"<<vtime<<endl;
    cout<<"==============================================================================="<<endl;
}

void XManager::ShowInfoToFile(const char *filename)
{
    static double startTime = Timer::GetTime();
    double diff =  Timer::GetTime() - startTime;
    double mem = (20.0*(NodeCount() + GetCacheCount()) + 4.0*GetUTableCount()) / ONE_MILLION;    
     
    fstream fs;
    fs.open(filename, ios::out | ios::app);
     
    fs<<"  NC:"<<XFunction::Format(NodeCount(),10);
    fs<<", CC:"<<XFunction::Format(GetCacheCount() / ONE_MILLION, 6);
    fs<<", Mem:"<<XFunction::Format(mem,10);
    fs<<", Hit:"<<XFunction::Format(Get_CTable_HitRate(),10,3);
    fs<<", Time:"<<XFunction::Format(diff,10,3);
    
    fs<<endl;
    
    fs.close();
}

void XManager::ShowInfoToFile(const char *filename, const string &info)
{
    fstream fs;
    fs.open(filename, ios::out | ios::app);
    fs<<info<<endl;
    fs.close();
}


    
