// #include<stdio.h>
#include"prog_syscall.h"
int result[10]={1,2,3,4,5,6,7,8,9,10};

//result:11 12 13 14 15 1 2 3 4 5

int main()
{
	int i=0;
	for(i=0;i<5;i++)
		result[i]=result[i]+10;
	for(i=5;i<10;i++)
		result[i]=result[i]-5;
	if(result[0]==11 && result[1]==12)
		prog_syscall(0);
	return 0;
}