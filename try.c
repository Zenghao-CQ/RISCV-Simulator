#include<stdio.h>
int main()
{
    char a[4]= {0x1,0x2,0x3,0x4};
    int x = *((int*) a);
    printf("%x",x);
}