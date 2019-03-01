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



#ifndef __XINTS__
#define __XINTS__

#include <memory.h>
#include <iostream>
using namespace std;

class XInts{
private:
    int size; 
    int *buffer;
public:
    XInts():size(0),buffer(0){};
    ~XInts(){
        size  = 0; 
        if(buffer){
            free(buffer);
            buffer = NULL;
        };
    }
    inline void set_size(int vsize);
    inline int  get_size();
    inline void set_value(int index, int value);
    inline int  get_value(int index);
};

inline void XInts::set_size(int vsize)
{
    if(buffer){
        free(buffer);
        buffer = NULL;
    }
    size = vsize;

    buffer = (int *)realloc(buffer, size * sizeof(int));
    memset(buffer, 0, size * sizeof(int));
}

inline int  XInts::get_size()
{
    return size;
}

inline void XInts::set_value(int index, int value)
{    
    if(index >= size){      
        int addCount = (index - size)+1000;
        buffer = (int *)realloc(buffer, (size + addCount)*sizeof(int));
        memset(&buffer[size], 0, addCount * sizeof(int));
        size += addCount;
    }
    buffer[index] = value;
}

inline int  XInts::get_value(int index)
{
    if(index >= size) return 0;
    return buffer[index];
}

#endif

