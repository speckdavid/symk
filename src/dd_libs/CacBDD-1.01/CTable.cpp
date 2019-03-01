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

#include "Base.h"
#include "CTable.h"
#include "Manager.h"
#include "Timer.h"
#include <iostream>
using namespace std;

XCTable::XCTable(XManager *manager, int vBitCount)
{
    mgr = manager;    
    count = (1 << vBitCount); 
    shiftSize = 32 - vBitCount;
    
    items = new CTableNode[count];
    
    toAddCount = 0;
    findCount = 0;
    foundedCount = 0;
    missCount = 0;
} 
  
XCTable::~XCTable()
{
    delete []items;
} 

void XCTable::Clear()
{
    for(int i=0; i<count; i++){   
        items[i].SetValue(0,0,0,0);        
    }
    toAddCount = 0;    
    findCount = 0; 
    foundedCount = 0;
    missCount = 0;
}

void XCTable::Refresh()
{
    Clear();
    
    for(int i=1; i<mgr->NodeCount(); i++){
        int v = mgr->nodes[i].var;
        DD  t = mgr->nodes[i].Then;
        DD  e = mgr->nodes[i].Else;
        Insert(v, t, e, i);

    }    
}
 
void XCTable::Expand(bool isInc)
{
    int newcount = count << 1;
    if(!isInc) newcount = count >> 1;

    CTableNode *newitems = new CTableNode[newcount];
    int newshiftSize = shiftSize - 1;
    if(!isInc) newshiftSize = shiftSize + 1;
    
    for(int i=0; i<count; i++){
        int pos = Hash3(items[i].A, items[i].B, items[i].C, newshiftSize); 
        newitems[pos].A = items[i].A;
        newitems[pos].B = items[i].B;
        newitems[pos].C = items[i].C;
        newitems[pos].R = items[i].R;  
    }
    delete []items;
    items = newitems;
    count = newcount;
    shiftSize = newshiftSize;
    toAddCount = 0;
}

bool XCTable::Find(DD A, DD B, DD C, DD &r)
{	
    findCount++;
    int pos = Hash3(A, B, C, shiftSize); 
    if ((A == items[pos].A)&&(B == items[pos].B)&&(C == items[pos].C)){	
        foundedCount++;
        r = items[pos].R;
        return true;
    }
    missCount++;
    return false;
}

void XCTable::Insert(DD A, DD B, DD C, DD tmp)
{	
    if(toAddCount > 0){
        Expand();  
    }
    //-------------------------------------------------------
    int pos = Hash3(A, B, C, shiftSize); 
    items[pos].A = A;
    items[pos].B = B;
    items[pos].C = C;
    items[pos].R = tmp;  
}

void  XCTable::size_inc(int incCnt)
{     
    if(toAddCount <= 0 && count <= mgr->maxCacheSize / 2 && mgr->GetAvailMem() > (long long)(count * sizeof(CTableNode))){        
        toAddCount += incCnt;                    
    }
}

