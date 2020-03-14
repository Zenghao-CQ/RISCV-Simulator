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
    //ctrl_BUBBLE_DI = true;
    //ctrl_BUBBLE_WB = true;
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
    //if(ctrl_BUBBLE_DI == true)
    //{
    //    ctrl_BUBBLE_DI = false;
    //    ctrl_BUBBLE_WB = true;//!!import
    //    return -2;//mean last instr is jump
    //}
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
                printf("\t!!!ERROR CODE in opcode_i,func3=5,func7=%x\n",func7);
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
            printf("\tandi %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
    }//endif op 0x13
    else if(opcode == OPCODE_I_4)//jalr
    {
        int rd = get_rd(inst);
        int rs1 = get_rs1(inst);
        int imm = get_imm_i(inst);
        //set PC
        PC_NEXT = (regs[rs1] + imm)&0xfffffffe;
        //set rd
        ctrl_wb_REG = true;
        wb_REG_No = rd;
        wb_REG_val = PC;//!!!when decode PC point to next instrct
        //bubble
        //ctrl_BUBBLE_DI = true;
        //ctrl_BUBBLE_WB = true;
        printf("\tjalr %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
    }
    else if(opcode == OPCODE_U_1)//auipc
    {
        int rd = get_rd(inst);
        int off = get_imm_u(inst);
        ctrl_wb_REG = true;
        wb_REG_No = rd;
        wb_REG_val = PC - 4 + off;//its not right,next PC is =4? no jump
        printf("\tauipc %s,%x\n",regnames[rd],off>>12);
    }
    else if(opcode ==  OPCODE_U_2)//LUI
    {
        int rd = get_rd(inst);
        int off = get_imm_u(inst);
        ctrl_wb_REG = true;
        wb_REG_No = rd;
        wb_REG_val = off;
        printf("\tlui %s,%x\n",regnames[rd],off>>12);
    }
    else if(opcode = OPCODE_UJ)//jal !!brach wrong bubble
    {
        int rd = get_rd(inst);
        int imm = get_imm_uj(inst);
        //set PC
        PC_NEXT = PC - 4 + imm;//!!!when decode PC point to next instrct
        //set rd
        ctrl_wb_REG = true;
        wb_REG_No = rd;
        wb_REG_val = PC;//!!!when decode PC point to next instrct
        //bubble
        //ctrl_BUBBLE_DI = true;
        //ctrl_BUBBLE_WB = true;
        printf("\tjal %s,%x\n",regnames[rd],PC_NEXT);//!!not val in dump
    }
    else if(opcode == OPCODE_SB)
    {
        int func3 = get_funct3(inst);
        int rs1 = get_rs1(inst);
        int rs2 = get_rs2(inst);
        int off = get_imm_sb(inst);
        if(func3 == 0X0)//beq
        {
            if(regs[rs1] == regs[rs2])
                PC_NEXT = off + PC - 4;
            printf("\tbeq %s,%s,imm",regnames[rs1],regnames[rs2],off + PC - 4);
        }
        else if(func3 == 0X1)//bne
        {
            if(regs[rs1] != regs[rs2])
                PC_NEXT = off + PC - 4;
            printf("\tbne %s,%s,imm",regnames[rs1],regnames[rs2],off + PC - 4);
        }
        else if(func3 == 0X4)//blt
        {
            if(regs[rs1] < regs[rs2])
                PC_NEXT = off + PC - 4;
            printf("\tblt %s,%s,imm",regnames[rs1],regnames[rs2],off + PC - 4);
        }
        else if(func3 == 0X5)//bge
        {
            if(regs[rs1] >= regs[rs2])
                PC_NEXT = off + PC - 4;
            printf("\tbge %s,%s,imm",regnames[rs1],regnames[rs2],off + PC - 4);
        }
        else if(func3 == 0X6)//bltu
        {
            if((uint64_t)(regs[rs1]) < (uint64_t)(regs[rs2]))
                PC_NEXT = off + PC - 4;
            printf("\tbltu %s,%s,imm",regnames[rs1],regnames[rs2],off + PC - 4);
        }
        else if(func3 == 0X7)//bgeu
        {
            if((uint64_t)(regs[rs1]) >= (uint64_t)(regs[rs2]))
                PC_NEXT = off + PC - 4;
            printf("\tbltu %s,%s,imm",regnames[rs1],regnames[rs2],off + PC - 4);
        }
        else
        {
            printf("\t!!!ERROR CODE in opcode_sb,func3=%x\n",func3);
            return -1;
        }
    }
    
    
    else
    {
        printf("###kunown instruct:%8x",inst);
        return -1;
    }    
    return 0;
}
int write_back()
{

}