#include<stdio.h>
#include<stdint.h>

#define to_uint64(a) *((int32_t*)&a)
int main()
{
    uint32_t a = 0xffffffff;
    uint32_t b = a>>1;
    printf("%ux\n",b);
}