#include<stdio.h>
#include<stdint.h>

#define to_uint64(a) *((int32_t*)&a)
int main()
{
    int32_t a = 0xffffffff;
    int32_t b = 0x10000000;
    
    uint32_t x = (uint32_t)(a);
    uint32_t y = (uint32_t)(b);
    
    if(a<b)
    printf("shit\n");
    if(x<y)
    printf("shit\n");
}