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

#include "UTable.h"
#include "Manager.h"
#include <cmath>
#include <iostream>
using namespace std;

XUTable::XUTable(XManager *manager, int vBitCount)
{
    mgr = manager;     
    count = (1 << vBitCount); 
    shiftSize = 32 - vBitCount;

    items = new DD[count];
    memset(items, 0, sizeof(DD) * count);

    findCount = 0;
    foundedCount = 0;
}

XUTable::~XUTable()
{		
    delete []items;
}

void XUTable::Clear()
{
    memset(items, 0, sizeof(DD) * count);
    findCount = 0;
    foundedCount = 0;
}

void XUTable::Refresh() 
{
    memset(items, 0, sizeof(DD) * count);  
  
    for(int i=1; i<mgr->NodeCount(); i++){
        int v = mgr->nodes[i].var;
        DD  t = mgr->nodes[i].Then;
        DD  e = mgr->nodes[i].Else;
        int pos = Hash3(v, t, e, shiftSize); 
        mgr->nodes[i].var  = v;
        mgr->nodes[i].Then = t;
        mgr->nodes[i].Else = e;
        mgr->nodes[i].Next = items[pos];
        items[pos] = i;
    }
    
    findCount = 0;
    foundedCount = 0;    
}

void  XUTable::Expand()
{    
    count = count << 1;    
    
    delete []items;
    items = new DD[count];
    memset(items, 0, sizeof(DD) * count);

    shiftSize = shiftSize - 1;    
    for(int i=1; i<mgr->NodeCount(); i++){
        int v = mgr->nodes[i].var;
        DD  t = mgr->nodes[i].Then;
        DD  e = mgr->nodes[i].Else;
        int pos = Hash3(v, t, e, shiftSize); 
        mgr->nodes[i].var  = v;
        mgr->nodes[i].Then = t;
        mgr->nodes[i].Else = e;
        mgr->nodes[i].Next = items[pos];
        items[pos] = i;
    }
    
    if(mgr->adjustCTableType == CACHE_FIX || mgr->adjustCTableType == CACHE_OLD){
        mgr->CTable->size_inc(1);
    }
}

DD XUTable::Find_or_Add_Unique_Table(int v, DD A, DD B)
{	
    if((mgr->NodeCount()>>2) > count){ Expand(); }
    
    findCount++;
    bool IsNot = false;    

    if (ISCOMP(A)){
        A = mgr->Not(A);
        B = mgr->Not(B);
        IsNot = true;
    }
            
    int pos = Hash3(v, A, B, shiftSize);    
 
    DD cur = items[pos];            
    while(cur !=0 ){        
        if(cur > ADDR(A) && cur > ADDR(B)){
            if( mgr->nodes[cur].var == v  && mgr->nodes[cur].Then == A && mgr->nodes[cur].Else == B){      
                foundedCount++;
                if(IsNot) return mgr->Not(cur);
                else return cur;                
            }
            cur = mgr->nodes[cur].Next;            
        }
        else{            
            break;
        }
    }       
    int n = mgr->GetFreeNode();
            
    mgr->nodes[n].var  = v;
    mgr->nodes[n].Then = A;
    mgr->nodes[n].Else = B;
    mgr->nodes[n].Next = items[pos];
    items[pos] = n;
    if(IsNot) return mgr->Not(n);    
    return n;
}

double  XUTable::LinkLength()
{
    int c = 0;
    for(int i=0; i<count; i++){
        if(items[i] > 0) c++;
    }
    return (1.0 * mgr->NodeCount() / c);
}
