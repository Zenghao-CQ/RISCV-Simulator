#include<stdio.h>
#include<stdint.h>
int main()
{
    uint64_t a = 0xffffffff;
    uint64_t x = a>>8;
    int64_t tmp = (int64_t)a;
    a = a>>8;
    uint64_t y = (uint64_t)tmp;
    printf("%lu\n",sizeof(a));
    printf("%lx\n%lx\n",x,y);
}