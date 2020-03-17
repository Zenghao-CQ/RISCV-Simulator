#include"syscall.h"
int syscall(int no)
{
    printf("\n###############################\n");
    printf("#### sycall No:%d received! ####\n",no);
    printf("###############################\n\n");
    return 0;
}