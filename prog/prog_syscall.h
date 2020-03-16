int prog_syscall(int no)
{
    if(no == 0)
    {
        __asm__("li a0,0\n\tcall");
        return 0;
    }
    else if(no == 10)
    {
        __asm__("li a0,0\n\tcall");
        return 0;
    }
    else
        return -1;    
}