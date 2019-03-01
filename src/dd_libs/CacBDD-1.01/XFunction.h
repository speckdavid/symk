
#ifndef _FUNCTION_
#define _FUNCTION_

#include <stdio.h>
#include <cmath>
#include <iostream>
#include <string>
using namespace std;

class XFunction{
public:
    static float  format_float(float v, int dotCnt) //dotCnt 表示小数点后保留几位
    {
        if(dotCnt > 6) dotCnt = 6;

        float k = pow(10.0f, dotCnt);
        float i = floor(v * k) / k;
        return i;
    }

    static double  format_double(double v, int dotCnt) //dotCnt 表示小数点后保留几位
    {
        if(dotCnt > 6) dotCnt = 6;

        double k = pow(10.0f, dotCnt);
        double i = floor(v * k) / k;
        return i;
    }

    static string Format(int v, int outlen)
    {
        char buf[50];
        memset(buf, 0, 50);
        sprintf(buf, "%d", v);
        string r = buf;
        size_t k = r.length();
        for(int i=0; i<outlen-k; i++){
            r = r+' ';
        }                
        return r;
    }

    static string Format(long long v, int outlen)
    {        
        char buf[50];
        memset(buf, 0, 50);
        sprintf(buf, "%lld", v);
        string r = buf;
        size_t k = r.length();
        for(int i=0; i<outlen-k; i++){
            r = r+' ';
        }                
        return r;
    }

    static string Format(double v, int outlen, int dotlen = 2)
    {
        v = format_double(v, dotlen);
        char buf[50];
        memset(buf, 0, 50);
        sprintf(buf, "%f", v);
        string r = buf;
        int k = (int)r.length();
        for(int i=0; i<outlen-k; i++){
            r = r+' ';
        }                
        return r;
    }
    
    static string Format(const char *v, int slen, int outlen)    
    {
        if(outlen < slen){ outlen = slen; }
        
        string r = v;
        for(int i=0; i<outlen - slen; i++){
            r = r + ' ';
        }
        return r;
    }
};

#endif
