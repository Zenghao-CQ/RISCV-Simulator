#include"cpu.h"
#include"elf.h"
#include"pipline.h"

/*
    for load memory test, we just excute "main" section\
    cause other section contain 16Byte code
*/
void print_main_code()
{
    for(int addr = madr;addr <= endmain; addr+=4)
    {
        if(fetch_instr(addr)!=0)
        {
            printf("\n##Invalid PC at %x",addr);
        }
        printf("%x: %8x\n",addr,IR);
    }
}
/*
    run simulaotr
*/
int main()
{
    //load_memory("./add");
    //print_main_code();
    INSTR tmp;
    while (1)
    {
       scanf("%x",&tmp);
       decode_excute(tmp);
    }
    
}