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
    if(load_memory("./add")!=0)
    {
        printf("\n***Load memory false!\n");
        return -1;
    }
    init();
    while (PC != endmain)
    {
        ctrl_wb_REG = false;
        ctrl_wb_MEM = false;
        //print_all_REGS();
        if(fetch_instr()!=0) 
        {
            printf("\n***fetch instruction false!\n");
            return -1;  
        }
        PC += 4;//!! in decode pC point to next PC
        if(decode_excute(IR)!=0)
        {
            printf("\n***decode and excute false!\n");
            return -1; 
        }
        PC -= 4;//!! recover PC from decode(when pipline PC point to next instrucion)
        if(write_back()!=0)
        {
            printf("\n***write back false!\n");
            return -1; 
        }
    }
    
}