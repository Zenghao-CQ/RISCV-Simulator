#include<stdio.h>
#include<stdint.h>

#define to_uint64(a) *((int32_t*)&a)
int main()
{
    int8_t a[3]={1,2,3};
    int16_t b = *((int16_t *) (&a[0]));
    printf("%x\n",b);
}