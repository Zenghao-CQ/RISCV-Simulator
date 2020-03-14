#include"pipline.h"
#include"cpu.h"

/***hardware***/
unsigned int PC;
unsigned int END;
unsigned int PC_NEXT;
INSTR IR;
/***control***/
//!!!its importent to reset it 
bool ctrl_wb_REG;//if write back reg file,
bool ctrl_wb_MEM;//if write back memory
int wb_REG_No;
int wb_MEM_off;
REG wb_REG_val;//uint64_t
REG wb_MEM_val;
bool ctrl_BUBBLE_DI;//if branch predict is wrong
bool ctrl_BUBBLE_WB;//if branch predict is wrong

/***exculte part***/
void init()
{
    PC = madr;//inital PC start from "main"
    END = endmain;//end of "main"
    regs[3] = gp;
    regs[2] = MEM_SIZE/2;
    regs[0] = 0;
    ctrl_wb_MEM = false;
    ctrl_wb_REG = false;
    //!!this is importent cause in 1st cycle pipline is empty
    ctrl_BUBBLE_DI = true;
    ctrl_BUBBLE_WB = true;
}
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
    printf("\n###load memory success!\n");
    return 0;
}
/***stage 1,fetch intruction from mem,will never bubble***/
int fetch_instr(int PC)//addr of bytes
{
    //PC = PC_NEXT;//!!test version, should use later
    if(PC%4!=0)//iligal addr
    {
        printf("\n###invalid PC");
        return -1;
    }
    IR = *((INSTR*) (memory + PC));
    //!!need modify when bubble later
    PC_NEXT = PC+4;
    return 0;
}
int decode_excute(INSTR inst)
{
    printf("###code: %x\n",inst);
    INSTR opcode = get_opcode(inst);
    //printf("\topcode: %x",opcode);
    if(opcode == OPCODE_I_2)//0x13
    {
        int func3 = get_funct3(inst);
        if(func3 == 0x0)//addi
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int imm = get_imm_i(inst);
            ctrl_wb_REG =true;
            wb_REG_No = rd;
            wb_REG_val = regs[rs1]+imm;
            printf("\taddi %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x1)//slli
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int imm = get_imm_i(inst)&0x1f;//lower 5 bit
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = regs[rs1]<<imm;
            printf("\tslli %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x2)//slti
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int imm = get_imm_i(inst);
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = (regs[rs1]<imm) ? 1:0;
            printf("\tslti %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x4)//xori
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int imm = get_imm_i(inst);
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = regs[rs1]^imm;
            printf("\txori %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x5)//srli&srai
        {
            //!! note that REG is uint64
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int imm = get_imm_i(inst)&0x1f;//lower 5 bit
            int func7 = get_funct7(inst);
            if(func7 = 0x0)//srli 0 
            {
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = regs[rs1]>>imm;
                printf("\tsrli %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
            }
            else if(func7 == 0x10)//srai 0|1
            {
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                int64_t tmp = (int64_t)regs[rs1];
                tmp = tmp >> imm;
                wb_REG_val = (REG)tmp;
                printf("\tsrai %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
            }
            else
            {
                printf("\t!!!ERROR CODE in opcode_i,func3=5\n");
                return -1;
            }            
        }
        else if(func3 == 0x6)//ori
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int imm = get_imm_i(inst);
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = regs[rs1] | imm;
            printf("\tori %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x7)//andi
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int imm = get_imm_i(inst);
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = regs[rs1] & imm;
            printf("\tori %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
    }//endif op 0x13
    else
    {
        printf("###kunown instruct:%x",inst);
        return -1;
    }    
    return 0;
}
int write_back()
{

}