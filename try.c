#include<stdio.h>
#include<stdint.h>

#define to_uint64(a) *((int32_t*)&a)
int main()
{
    int64_t a = 0xfffffffff - 2 , b = 1;
    int64_t c = a+b;
    int t = c;
    int64_t d = t;
    printf("%llx %x %llx\n",c,t,d);
}