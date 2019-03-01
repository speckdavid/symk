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


#ifndef _DDNODE_
#define _DDNODE_

#include "Base.h"
#include <stdlib.h>
#include <iostream>
#include <assert.h>

using namespace std;

class DdNode{
public:
    int var;     
    int Then;
    int Else;	
    int Next;	
    DdNode():var(0),Then(0),Else(0),Next(0){};
    DdNode(DdNode &v)
    {  
        var = v.var;
        Then = v.Then;
        Else = v.Else;
        Next = v.Next;
    } 
    DdNode& operator = (const DdNode& v)
    {
        var = v.var;
        Then = v.Then;
        Else = v.Else;
        Next = v.Next;
        return *this;
    }

    void SetValue(int a, int b, int c, int d){ var = a; Then = b; Else = c; Next = d; };
};

class DdNodes{
private:
    friend class XManager;
    int slotCount; 
    int slotSize; 
    int nodeCount; 
    DdNode **slots;
public:
    DdNodes();
    ~DdNodes();

    int  GetFreeNode();
    void Init(int vSlotSize);
    void Clear();
    int  NodeCount(){ return nodeCount;};

    DdNode& operator [] (int index);
};

inline DdNode& DdNodes::operator [] (int index)
{
    int s = index / slotSize;
    int i = index % slotSize;    
    return slots[s][i];
}

inline int DdNodes::GetFreeNode() 
{   
    if(nodeCount == slotCount * slotSize){      
        slots[slotCount] = new DdNode[slotSize];

        for(int k=0; k<slotSize; k++){
            slots[slotCount][k].SetValue(0,0,0,0);
        }                
        slotCount++;        

    }
    nodeCount++;
    return (nodeCount - 1);
};

#endif

