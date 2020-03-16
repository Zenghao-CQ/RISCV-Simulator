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
int wb_MEM_len;//bytes length
int64_t wb_REG_val;//uint64_t
uint64_t wb_MEM_val;
bool ctrl_BUBBLE_DI;//if branch predict is wrong
bool ctrl_BUBBLE_WB;//if branch predict is wrong

/***load memrory\reg***/
int8_t get_byte(int addr)
{
    return (int8_t)memory[addr];
}
int16_t get_half(int addr)
{
    return *((int16_t *) (&memory[addr]));
}
int32_t get_word(int addr)
{
    return *((int32_t *) (&memory[addr]));
}
int64_t get_double(int addr)
{
    return *((int64_t *) (&memory[addr]));
}
int64_t get_reg(int no)
{
    if(no = 0)
        return 0;
    else
        return regs[no];
}
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
    printf("instruct: 0x%08x\n",inst);
    INSTR opcode = get_opcode(inst);
    printf("\topcode: 0x%x",opcode);

    if(opcode == OPCODE_I_1)//load memry
    {
        int rd = get_rd(inst);
        int rs1 = get_rs1(inst);
        int off = get_imm_i(inst);
        int64_t addr = get_reg(rs1) + off;
        int func3 = get_funct3(inst);
        if(func3 == 0)//lb
        {
            int8_t val = get_byte(addr);//get byte
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = (int64_t)val;//!!sign extend!!
            printf("\tlb %s,%s,%d\n",regnames[rd],regnames[rs1],off);
        }   
        else if(func3 == 1)//lh
        {
            int16_t val = get_half(addr);
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = (int64_t)val;//!!sign extend!!
            printf("\tlh %s,%s,%d\n",regnames[rd],regnames[rs1],off);
        }
        else if(func3 == 2)//lw
        {
            int32_t val = get_word(addr);
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = (int64_t)val;//!!sign extend!!
            printf("\tlw %s,%s,%d\n",regnames[rd],regnames[rs1],off);
        }
        else if(func3 == 3)//ld
        {
            int64_t val = get_double(addr);
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = (int64_t)val;//!!sign extend!!
            printf("\tld %s,%s,%d\n",regnames[rd],regnames[rs1],off);
        }
        else
        {
            printf("\t!!!ERROR CODE in load,func3=0x%x\n",func3);
            return -1;
        }
    }
    else if(opcode == OPCODE_I_2)//0x13
    {
        int func3 = get_funct3(inst);
        if(func3 == 0x0)//addi
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int64_t imm = get_imm_i(inst);//sign extend
            ctrl_wb_REG =true;
            wb_REG_No = rd;
            wb_REG_val = get_reg(rs1) + imm;
            printf("\taddi %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x1)//slli
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int64_t imm = get_imm_i(inst)&0x1f;//lower 5 bit
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = get_reg(rs1) << imm;
            printf("\tslli %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x2)//slti
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int64_t imm = get_imm_i(inst);
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = (get_reg(rs1) < imm) ? 1 : 0;
            printf("\tslti %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x4)//xori
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int64_t imm = (int64_t) (get_imm_i(inst));//sign extend
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = get_reg(rs1)^imm;
            printf("\txori %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x5)//srli&srai
        {
            //!! note that REG is uint64
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int imm = get_imm_i(inst)&0x1f;//lower 5 bit
            int func7 = get_funct7(inst);
            if(func7 = 0x0)//srli 0 !!nosign
            {
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                uint64_t tmp = (uint64_t)(get_reg(rs1));//logic shift
                tmp = tmp >> imm;
                wb_REG_val = (int64_t)tmp;
                printf("\tsrli %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
            }
            else if(func7 == 0x10)//srai 0|1
            {
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                int64_t tmp = (int64_t)(get_reg(rs1));//alg shift
                tmp = tmp >> imm;
                wb_REG_val = (int64_t)tmp;
                printf("\tsrai %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
            }
            else
            {
                printf("\t!!!ERROR CODE in opcode_i,func3=5,func7=0x%x\n",func7);
                return -1;
            }            
        }
        else if(func3 == 0x6)//ori
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int64_t imm = (int64_t)(get_imm_i(inst));
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = get_reg(rs1) | imm;
            printf("\tori %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x7)//andi
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int64_t imm = (int64_t)(get_imm_i(inst));
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = get_reg(rs1) & imm;
            printf("\tandi %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
    }//endif op 0x13
    else if(opcode == OPCODE_I_4)//jalr
    {
        int rd = get_rd(inst);
        int rs1 = get_rs1(inst);
        int imm = get_imm_i(inst);
        //set PC
        PC_NEXT = (get_reg(rs1) + imm)&0xfffffffe;
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
        int64_t off = get_imm_u(inst);
        ctrl_wb_REG = true;
        wb_REG_No = rd;
        wb_REG_val = (int64_t)PC - 4 + off;//its not right,next PC is =4? no jump
        printf("\tauipc %s,0x%x\n",regnames[rd],off>>12);
    }
    else if(opcode ==  OPCODE_U_2)//LUI
    {
        int rd = get_rd(inst);
        int off = get_imm_u(inst);
        ctrl_wb_REG = true;
        wb_REG_No = rd;
        wb_REG_val = off;
        printf("\tlui %s,0x%x\n",regnames[rd],off>>12);
    }
    else if(opcode == OPCODE_UJ)//jal !!brach wrong bubble
    {
        int rd = get_rd(inst);
        int imm = (int64_t)get_imm_uj(inst);
        //set PC
        PC_NEXT = PC - 4 + imm;//!!!when decode PC point to next instrct
        //set rd
        ctrl_wb_REG = true;
        wb_REG_No = rd;
        wb_REG_val = PC;//!!!when decode PC point to next instrct
        //bubble
        //ctrl_BUBBLE_DI = true;
        //ctrl_BUBBLE_WB = true;
        printf("\tjal %s,0x%x\n",regnames[rd],PC_NEXT);//!!not val in dump
    }
    else if(opcode == OPCODE_S)//!!no sign extend
    {
        int rs1 = get_rs1(inst);
        int rs2 = get_rs2(inst);
        int off = get_imm_s(inst);//!!sign extend
        int64_t addr = get_reg(rs1) + off;
        int func3 = get_funct3(inst);
        if(func3 == 0)//sb
        {
            ctrl_wb_MEM = true;
            wb_MEM_off = addr;
            wb_MEM_len = 1;
            wb_MEM_val = (uint64_t)(get_reg(rs2)&0xff);
            printf("\tsb %s,%d(%s):0x%x",regnames[rs2],off,regnames[rs1],addr);
        }
        if(func3 == 1)//sh
        {
            ctrl_wb_MEM = true;
            wb_MEM_off = addr;
            wb_MEM_len = 2;
            wb_MEM_val = (uint64_t)(get_reg(rs2)&0xffff);
            printf("\tsh %s,%d(%s):0x%x",regnames[rs2],off,regnames[rs1],addr);
        }
        if(func3 == 2)//sw
        {
            ctrl_wb_MEM = true;
            wb_MEM_off = addr;
            wb_MEM_len = 4;
            wb_MEM_val = (uint64_t)(get_reg(rs2)&0xffffffff);
            printf("\tsw %s,%d(%s):0x%x",regnames[rs2],off,regnames[rs1],addr);
        }
        if(func3 == 3)//sd
        {
            ctrl_wb_MEM = true;
            wb_MEM_off = addr;
            wb_MEM_len = 8;
            wb_MEM_val = (uint64_t)get_reg(rs2);
            printf("\tsh %s,%d(%s):0x%x",regnames[rs2],off,regnames[rs1],addr);
        }
    }    
    else if(opcode == OPCODE_SB)
    {
        int func3 = get_funct3(inst);
        int rs1 = get_rs1(inst);
        int rs2 = get_rs2(inst);
        int off = get_imm_sb(inst);
        if(func3 == 0X0)//beq
        {
            if(get_reg(rs1) == get_reg(rs2))
                PC_NEXT = off + PC - 4;
            printf("\tbeq %s,%s,0x%x",regnames[rs1],regnames[rs2],off);
        }
        else if(func3 == 0X1)//bne
        {
            if(get_reg(rs1) != get_reg(rs2))
                PC_NEXT = off + PC - 4;
            printf("\tbne %s,%s,0x%x",regnames[rs1],regnames[rs2],off);
        }
        else if(func3 == 0X4)//blt
        {
            if(get_reg(rs2) < get_reg(rs2))
                PC_NEXT = off + PC - 4;
            printf("\tblt %s,%s,0x%x",regnames[rs1],regnames[rs2],off);
        }
        else if(func3 == 0X5)//bge
        {
            if(get_reg(rs1) >= get_reg(rs2))
                PC_NEXT = off + PC - 4;
            printf("\tbge %s,%s,0x%x",regnames[rs1],regnames[rs2],off);
        }
        else if(func3 == 0X6)//bltu
        {
            if((uint64_t)(get_reg(rs1)) < (uint64_t)(get_reg(rs2)))
                PC_NEXT = off + PC - 4;
            printf("\tbltu %s,%s,0x%x",regnames[rs1],regnames[rs2],off);
        }
        else if(func3 == 0X7)//bgeu
        {
            if((uint64_t)(get_reg(rs1)) >= (uint64_t)(get_reg(rs2)))
                PC_NEXT = off + PC - 4;
            printf("\tbltu %s,%s,0x%x",regnames[rs1],regnames[rs2],off);
        }
        else
        {
            printf("\t!!!ERROR CODE in opcode_sb,func3=0x%x\n",func3);
            return -1;
        }
    }
    
    
    else
    {
        printf("\nERROR: unknown opcode,unkown instruct:%08x\n",inst);
        return -1;
    }    
    return 0;
}
int write_back()
{

}