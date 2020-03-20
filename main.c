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
    int cycnum = 0;
    int val_instr = 0;
    init();
    bool GO = false;
    while (PC != endmain)
    {
        cycnum += 3;
        val_instr++;
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
        //PC += 4;//!! in decode pC point to next PC
        if(decode_excute(IR)!=0)
        {
            printf("***decode and excute false!\n");
            return -1; 
        }
        //PC -= 4;//!! recover PC from decode(when pipline PC point to next instrucion)
        if(write_back()!=0)
        {
            printf("***write back false!\n");
            return -1; 
        }
    }    
    printf("Simulate success!\n");
    printf("Cycle number:%d\n",cyclenum);
    printf("Instruction number:%d\n",val_instr);
    return 0;    
}
#endif //SINGLE

#ifdef PIPE
int main(int argc, char* argv[])//if jal or brach, 
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
    bool old_ctrl_wb_REG;
    bool old_ctrl_wb_MEM;
    int old_wb_REG_No;
    int old_wb_MEM_off;
    int old_wb_MEM_len;
    data_conflict = false;
    init();
    int cyclenum = 0;
    int val_instr = 0;
    int data_con = 0;
    int ctrl_con = 0;

    while (PC != endmain)
    {      
        cyclenum++;
        val_instr++;
        if(write_back()!=0)
        {
            printf("***write back false!\n");
            return -1; 
        }
        old_ctrl_wb_MEM = ctrl_wb_MEM;
        old_ctrl_wb_REG = ctrl_wb_REG;
        old_wb_MEM_len = wb_MEM_len;
        old_wb_MEM_off = wb_MEM_off;
        old_wb_REG_No = wb_REG_No;
        //set
        ctrl_wb_MEM = false;
        ctrl_wb_REG = false;
        ctrl_read_REG = false;
        ctrl_read_MEM = false;
        data_conflict = false;
        //PC+=4;
        if(decode_excute(IR)!=0)
        {
            printf("***decode and excute false!\n");
            return -1; 
        }
        //PC-=4;
        control_conflict = false;
        //if data conflict
        if(old_ctrl_wb_REG == true && ctrl_read_REG == true && old_wb_REG_No == read_REG_No)
        {
            data_con++;
            cyclenum++;
            data_conflict = true;
        }
        if(old_ctrl_wb_MEM && ctrl_read_MEM && 
            !((read_MEM_off + read_MEM_len < old_wb_MEM_off) || (old_wb_MEM_off + old_wb_MEM_len < read_MEM_off))
            )
        {
            data_con++;
            cyclenum++;
            data_conflict = true;
        }
        if(PC_NEXT != PC + 4)
        {
            ctrl_con++;
            cyclenum++;
            control_conflict = true;
        }
        if(fetch_instr()!=0) 
        {
            printf("***fetch instruction false!\n");
            return -1;  
        }
    }
    //last instruction
    //PC+=4;
    if(decode_excute(IR)!=0)
        {
            printf("***decode and excute false!\n");
            return -1; 
        }
    //PC-=4;
    if(write_back()!=0)
        {
            printf("***write back false!\n");
            return -1; 
        }
    //
    printf("Simulate success!\n");
    printf("Cycle number:%d\n",cyclenum);
    printf("Instruction number:%d\n",val_instr);
    printf("Data confilict:%d\n",data_con);
    printf("Control confilict:%d\n",ctrl_con);
    
    return 0;    
}
#endif //PIPE