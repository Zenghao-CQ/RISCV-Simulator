#include"syscall.h"
int syscall(int no)
{
    printf("###sycall No:%d received!\n",no);
    return 0;
}