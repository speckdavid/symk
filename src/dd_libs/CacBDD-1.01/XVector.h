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


#ifndef __XVECTOR__
#define __XVECTOR__

#include <stdlib.h>
#include <new>
using namespace std;

template <class T> class XVector
{
private:
    int _size;
    int _capacity;

public:
    T * data;

    T& operator[] (int index) const;
    XVector<T>& operator = (const XVector<T>& other);
    int  size() const { return _size; };

    void init(int vSize); 
    void resize(int vSize); 
    void shrink(); 
    void expand(int cnt); 
    T&   back();
    void pop_back();
    void push_back(const T& key);
    void insert(int index, T& key);
    void clear();
    void erase(int index); 
    void remove(T &x);
    int  find(T &x);
    int  capacity();    
    void Print();

    XVector();
    XVector(XVector<T>& other);
    XVector(int size);    
    ~XVector();
};

template<class T> XVector<T>::XVector()
{
    data = NULL;
    _size   = 0;
    _capacity  = 0;    
}

template<class T> XVector<T>::XVector(int vSize)
{
    init(vSize);
}

template<class T> void XVector<T>::init(int vSize)
{
    clear();

    data = NULL;
    _size = vSize;
    _capacity = vSize;
    data = (T*)realloc(data, _size * sizeof(T));  
    for(int i=0; i<_size; i++){
        new (&data[i]) T();        
    }
}

template<class T> void XVector<T>::resize(int vSize)
{
    if(vSize > _size){
        for(int i=_size; i<vSize; i++){
            T t;
            push_back(t);
        }
        _size = vSize;
    }
    else if(vSize < _size){
        for(int i=0; i<_size - vSize; i++){
            pop_back();
        }
        _size = vSize;        
    }        
}

template<class T> XVector<T>::XVector(XVector<T>& other)
{
    data = NULL;
    data = (T*)realloc(data, other.size() * sizeof(T));

    for(int i=0;i<other.size();i++){
        data[i] = other.data[i];
    }
    _size = other.size();
    _capacity = _size;
}

template<class T> XVector<T>::~XVector()
{
    clear();
}

template<class T> inline void XVector<T>::clear()
{
    if (data != 0)
    {
        for (int i = 0; i < _size; i++)
        {
            data[i].~T();

        }        
        _size = 0;
        free(data);        
        data = 0;
        _capacity = 0;
    }
}

template<class T> inline int XVector<T>::capacity()
{
    return _capacity;
}

template<class T> inline void XVector<T>::shrink()
{
    _capacity = _size;
    data = (T*)realloc(data, _capacity * sizeof(T));
}

template<class T> inline void XVector<T>::expand(int cnt)
{
    _capacity += cnt;
    data = (T*)realloc(data, _capacity * sizeof(T));
}

template<class T> void XVector<T>::insert(int x, T& key)
{
    if(_size == 0){
        push_back(key);
    }
    else{
        if (_size == _capacity)
        {
            _capacity = (2 > ((_capacity*3+1)>>1))?2:((_capacity*3+1)>>1);
            data = (T*)realloc(data, _capacity * sizeof(T));
        }
        _size++;
        new (&data[_size-1]) T();

        if(x >= _size-1) push_back(key);
        else{
          for(int i= _size-1;i>=x;i--){
             data [i+1] = data[i];
          }
          data[x] = key;
        }
    }
}

template<class T>inline  void XVector<T>::remove(T &x)
{
    for ( int i=0; i<_size; i++){
        if (data[i] == x){
            data[i].~T();
            data[i] = data[_size-1];
            data[_size-1].~T(); 
            _size--;            
            break;
        }
    }
}

template<class T>inline void XVector<T>::erase(int index)
{    
    if(index == _size-1) pop_back();
    else
    {
        data[index].~T();
        data[index] = data[_size-1];
        data[_size-1].~T();
        _size--;        
    }
}

template<class T>inline void XVector<T>::push_back(const T& elem)
{
    if(_size == 0){
        _capacity = _size = 1;
        data = (T*)realloc(data, _capacity * sizeof(T));
        new (&data[_size-1]) T();
        data[_size-1] = elem;
    }
    else{
        if (_size == _capacity){
            _capacity = (2 > ((_capacity*3+1)>>1))?2:((_capacity*3+1)>>1);            
            data = (T*)realloc(data, _capacity * sizeof(T));
        }
        new (&data[_size]) T();
        data[_size] = elem;
        _size++;
    }
}

template<class T> inline void XVector<T>::pop_back()
{
    if(_size > 0){
        _size--;
        data[_size].~T();        
    }
}

template<class T>inline T& XVector<T>::operator[] (int index) const
{ 
    return data[index]; 
}

template<class T>inline XVector<T>&  XVector<T>::operator = (const XVector<T>& other)
{
    if(size() > 0) clear();
    data = 0;
    if(_capacity < other.size()) data = (T*)malloc(other.size() * sizeof(T));
       
    for(int i=0; i<other.size(); i++){
        data[i] = other.data[i];
    }

    _size = other.size();
    _capacity = _size;

    return *this;
}

template<class T>inline T&  XVector<T>::back()
{
     return data[_size-1];
}

template<class T> inline int XVector<T>::find(T &x)
{
    for (int i = 0; i < _size; i++) {
        if (data[i] == x) return i;
    }
    return -1;
}

#endif
