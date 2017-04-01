#include "comm_struct/MachineBit.h"

int upper2lower(int c) 
{ 
    if (c >= 'A' && c <= 'Z') 
    { 
        return c + 'a' - 'A'; 
    } 
    else 
    { 
        return c; 
    } 
} 

uint64_t hex2int(char s[]) 
{ 
    int i; 
    uint64_t n = 0; 
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X')) //判断是否有前导0x或者0X
    { 
        i = 2; 
    } 
    else 
    { 
        i = 0; 
    } 
    for (; (s[i] >= '0' && s[i] <= '9') 
|| (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i) 
    {   
        if (upper2lower(s[i]) > '9') 
        { 
            n = 16 * n + (10 + upper2lower(s[i]) - 'a'); 
        } 
        else 
        { 
            n = 16 * n + (upper2lower(s[i]) - '0'); 
        } 
    } 
    return n; 
} 
