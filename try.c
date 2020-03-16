#include<stdio.h>
#include<stdint.h>

#define to_uint64(a) *((int32_t*)&a)
int main()
{
    int64_t a = INT64_MAX ,b = INT64_MAX;
    int64_t c = a*b;
    int64_t d = ((__int128_t)a*(__int128_t)b)>>64;
    printf("%llx %llx %llx %llx %llx\n",a,b,c,d,a*b);
}