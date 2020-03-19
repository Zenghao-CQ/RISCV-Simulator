#include"prog_syscall.h"
#define MSIZE 12
int a[MSIZE][MSIZE],b[MSIZE][MSIZE],c[MSIZE][MSIZE];
int main()
{
    for(int i = 0; i < MSIZE; ++i)
        for(int j = 0; j < MSIZE; ++j)
        {
            a[i][j] = i + j;
            b[i][j] = (i + j) % 2;
            c[i][j] = 0;
        }
    for(int i = 0; i < MSIZE; ++i)
        for(int j = 0; j < MSIZE; ++j)
        {
            for(int k = 0;k < MSIZE ; ++k)
            {
                int temp = a[i][k] * b[k][j];
                c[i][j] += temp;
            }
        }
    return 0;
}