#include"pipline.h"
#include"cpu.h"

/***hardware***/
unsigned int PC;
unsigned int PC_NEXT;
INSTR IR;

/***exculte part***/

int load_memory(char * filename)
{
    if (read_elf(filename) != 0)
    {
        printf("\n###Read ELF error\n");
        return -1;
    }
    FILE* excu_file = fopen(filename,"r");
    if(code_vadr + code_size >= MEM_SIZE || data_vadr + data_size > MEM_SIZE)
    {
        printf("\n###Out of memry size!\n");
        return -2;
    }
    fseek(excu_file, code_adr, 0);
    fread(&memory[code_vadr],1,code_size,excu_file);//load code segment

    fseek(excu_file, data_adr, 0);
    fread(&memory[data_vadr],1,data_size,excu_file);//load data segment
   
    fclose(excu_file);
    return 0;
}

int fetch_instr(int pc)//addr of bytes
{
    if(pc%4!=0)//iligal addr
    {
        printf("invalid PC");
        return -1;
    }
    IR = *((INSTR*) (memory + pc));
    return 0;
}
int decode_excute(INSTR inst)
{
    INSTR opcode = get_opcode(inst);
    printf("code: %x ",inst);
    printf("\topcode: %x",opcode);
    if(opcode == OPCODE_I_2)//0x13
    {
        int func3 = get_funct3(inst);
        if(func3 == 0x0)//addi
        {
            int imm = get_imm_i(inst);
            int rs1 = get_rs1(inst);
            printf("\t rs1 %d,imm %d\n",rs1,imm);
        }
    }//endif op 0x13
}