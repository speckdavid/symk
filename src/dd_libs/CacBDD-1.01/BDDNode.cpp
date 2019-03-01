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

#include "BDDNode.h"
#include "Manager.h"
#include <cstdio>

BDD::BDD(XManager *m, DD vNode)
{
    mgr = m;
    node = vNode;	
    m->externalBDDs.insert(this); 	
}

BDD::BDD(const BDD &right)
{    
   mgr = right.mgr;
   node = right.node;
   mgr->externalBDDs.insert(this);
}

BDD::BDD()
{
    mgr = 0;
    node = 0;
}

BDD::~BDD()
{
    if(mgr != NULL){
        mgr->externalBDDs.erase(this);
    }  	
}

XManager *BDD::manager() const
{
   return mgr;
}

DD BDD::Node() const
{
   return node;
}

BDD &BDD::operator=(const BDD& right) 
{	
    if (this == &right) return *this;
    node = right.node;
    mgr  = right.manager();
    mgr->externalBDDs.insert(this);
    return *this;
}

inline XManager *BDD::checkSameManager( const BDD &other) const 
{    
    if (mgr != other.mgr) {
        printf("\nERROR: Operands come from different manager.\n");      
    }
    return mgr;
} 

bool BDD::operator==(const BDD& other) const 
{
	checkSameManager(other);
    return node == other.node;
} 

bool BDD::operator!=(const BDD& other) const  
{
    this->checkSameManager(other);
    return node != other.node;
} 

BDD BDD::operator!() const 
{
    return BDD(mgr, mgr->Not(node));
} 

BDD BDD::operator+(const BDD& other) const   
{	
    XManager *mgr = this->checkSameManager(other);
    DD result = mgr->Or(node, other.node);
    
    return BDD(mgr, result);
} 

BDD BDD::operator*(  const BDD& other) const 
{
    XManager *mgr = this->checkSameManager(other);
    DD result = mgr->And(node, other.node);
    
    return BDD(mgr, result);
} 

BDD BDD::operator^(const BDD& other) const
{
    XManager *mgr = this->checkSameManager(other);
    DD result = mgr->Xor(node, other.node);
    
    return BDD(mgr, result);
} 

BDD BDD::operator<(const BDD& other) const
{
    XManager *mgr = this->checkSameManager(other);
    DD result = mgr->LessThan(node, other.node);
    
    return BDD(mgr, result);
}

BDD BDD::operator>(const BDD& other) const
{
	XManager *mgr = this->checkSameManager(other);
    DD result = mgr->MoreThan(node, other.node);
    
    return BDD(mgr, result);
}

BDD BDD::operator<=(const BDD& other) const
{
	XManager *mgr = this->checkSameManager(other);
    DD result = mgr->LessEqual(node, other.node);
    
    return BDD(mgr, result);
}

BDD BDD::operator>=(const BDD& other) const
{
	XManager *mgr = this->checkSameManager(other);
    DD result = mgr->MoreEqual(node, other.node);
    
    return BDD(mgr, result);
}

BDD BDD::operator%(const BDD& other) const
{
	XManager *mgr = this->checkSameManager(other);
    DD result = mgr->Nor(node, other.node);    
    return BDD(mgr, result);
}

BDD BDD::operator|(const BDD& other) const
{
	XManager *mgr = this->checkSameManager(other);
    DD result = mgr->Nand(node, other.node);    
    return BDD(mgr, result);
}

BDD BDD::operator&(const BDD& other) const
{
	XManager *mgr = this->checkSameManager(other);
    DD result = mgr->Xnor(node, other.node);    
    return BDD(mgr, result);
}

int BDD::Variable()
{
    return mgr->Variable(node);
}

BDD BDD::Then() const
{
    DD result = mgr->Then(node, mgr->Variable(node));
    return BDD(mgr, result);
}

BDD BDD::Else() const
{
    DD result = mgr->Else(node, mgr->Variable(node));
    return BDD(mgr, result);
}

BDD BDD::Exist(const BDD& cube)
{
    XManager *mgr = this->checkSameManager(cube);
    DD result = mgr->Exist(node, cube.node);    
    return BDD(mgr, result);
}

BDD BDD::Universal(const BDD& cube)
{
    XManager *mgr = this->checkSameManager(cube);
    DD result = mgr->Universal(node, cube.node);    
    return BDD(mgr, result);
}

BDD BDD::Restrict(const BDD& other) const
{
    XManager *mgr = this->checkSameManager(other);
    DD result = mgr->Restrict(node, other.node);    
    return BDD(mgr, result);
}

BDD BDD::Compose(int v, const BDD& other) const
{
    XManager *mgr = this->checkSameManager(other);
    DD result = mgr->Compose(node, v, other.node);
    return BDD(mgr, result);
}

BDD BDD::Permute(const vector<int> &permu) const
{
    int *x = new int[permu.size()];
    for(int i=0; i<permu.size(); i++){
        x[i] = permu[i];        
    }
    DD result = mgr->Permute(node, x);
    delete []x;
    return BDD(mgr, result);
}

BDD BDD::AndExist(const BDD& other, const BDD& cube)
{
    XManager *mgr = this->checkSameManager(other);
    DD result = mgr->AndExist(node, other.node, cube.node);
    return BDD(mgr, result);
}

void BDD::Support(vector<int> &vars)
{
    mgr->Support(node, vars);
}


