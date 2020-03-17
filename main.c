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
#ifdef SINGLE
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("No file input!\n");
        return -1;
    }
    char* filename = argv[1];
    if(load_memory(filename)!=0)
    {
        printf("***Load memory false!\n");
        return -1;
    }
    init();
    bool GO = false;
    while (PC != endmain)
    {
        while (!GO)
        {            
            char type;
            scanf("%c",&type);
            getchar();
            if(type == 'r')
            {
                print_all_REGS();
            }
            else if(type == 'm')
            {
                int off,len;
                scanf("%d %d",&off,&len);
                getchar();
                print_memory(off,len);
            }
            else if(type == 'c')
            {
                break;
            }
            else if(type == 'g')
            {
                GO = true;
                break;
            }            
            else
            {
                printf("%c no such command!\n",type);
            }            
        }
        

        ctrl_wb_REG = false;
        ctrl_wb_MEM = false;
        if(fetch_instr()!=0) 
        {
            printf("***fetch instruction false!\n");
            return -1;  
        }
        PC += 4;//!! in decode pC point to next PC
        if(decode_excute(IR)!=0)
        {
            printf("***decode and excute false!\n");
            return -1; 
        }
        PC -= 4;//!! recover PC from decode(when pipline PC point to next instrucion)
        if(write_back()!=0)
        {
            printf("***write back false!\n");
            return -1; 
        }
    }
    return 0;    
}
#endif //SINGLE