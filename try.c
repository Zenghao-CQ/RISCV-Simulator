#include<stdio.h>
#include<stdint.h>
#include<string.h>

#define to_uint64(a) *((int32_t*)&a)
int main()
{
    char buff[20];
    while(1)
    {
        scanf("%s",buff);
        if(strcmp(buff,"")==0)
            printf("empty\n");
        else
            printf("echo %s\n",buff);
    }
}