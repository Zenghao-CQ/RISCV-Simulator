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
        else
        {
            printf("%x: %08x\n",addr,IR);
            PC = addr + 4;
            decode_excute(IR);
        }
    }
}
/*
    run simulaotr
*/
int main()
{
    load_memory("./padd");
    print_main_code();    
}