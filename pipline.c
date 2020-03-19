#include"pipline.h"
#include"cpu.h"
#include"syscall.h"
#include<string.h>

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

bool ctrl_read_REG;
bool ctrl_read_MEM;
int read_REG_No;
int read_MEM_off;
int read_MEM_len;//bytes length
#ifdef PIPE
bool data_conflict;
bool control_conflict;
#endif
/***load memrory\reg***/
int8_t get_byte(int addr)
{
#ifdef PIPE
    ctrl_read_MEM =true;
    read_MEM_len = 1;
    read_MEM_off = addr;
#endif //PIPE
    return (int8_t)memory[addr];
}
int16_t get_half(int addr)
{
#ifdef PIPE
    ctrl_read_MEM =true;
    read_MEM_len = 2;
    read_MEM_off = addr;
#endif //PIPE
    return *((int16_t *) (&memory[addr]));
}
int32_t get_word(int addr)
{
#ifdef PIPE
    ctrl_read_MEM =true;
    read_MEM_len = 4;
    read_MEM_off = addr;
#endif //PIPE
    return *((int32_t *) (&memory[addr]));
}
int64_t get_double(int addr)
{
#ifdef PIPE
    ctrl_read_MEM =true;
    read_MEM_len = 8;
    read_MEM_off = addr;
#endif //PIPE
    return *((int64_t *) (&memory[addr]));
}
int64_t get_reg(int no)
{
    //printf("%%%red reg %d\n",no);
#ifdef PIPE
    ctrl_read_REG =true;
    read_REG_No = no;
#endif //PIPE
    if(no == 0)
        return 0;
    else
        return regs[no];
}
/***exculte part***/
void init()
{
    PC = madr - 4;//avoid false control conflict at first 
    PC_NEXT = madr;//inital PC start from "main"
    END = endmain;//end of "main"
    regs[3] = gp;
    regs[2] = MEM_SIZE/2;
    regs[0] = 0;
    ctrl_wb_MEM = false;
    ctrl_wb_REG = false;
    //!!this is importent cause in 1st cycle pipline is empty
    #ifndef SINGLE // dont write and decode at first
    ctrl_BUBBLE_DI = true;
    ctrl_BUBBLE_WB = true;
    #endif
    #ifdef PIPE 
    control_conflict = false;
    data_conflict = false;
    #endif
}
int load_memory(char * filename)
{
    if (read_elf(filename) != 0)
    {
        printf("\n***Read ELF error\n");
        return -1;
    }
    FILE* excu_file = fopen(filename,"r");
    if(code_vadr + code_size >= MEM_SIZE || data_vadr + data_size > MEM_SIZE)
    {
        printf("\n***Out of memry size!\n");
        return -2;
    }
    memset(memory,0,sizeof(memory));
    fseek(excu_file, code_adr, 0);
    fread(&memory[code_vadr],1,code_size,excu_file);//load code segment

    fseek(excu_file, data_adr, 0);
    fread(&memory[data_vadr],1,data_size,excu_file);//load data segment
    
    fclose(excu_file);
    printf("\n***load memory success!\n");
    return 0;
}
/***stage 1,fetch intruction from mem,will never bubble***/
int fetch_instr()//addr of bytes
{
    #ifdef PIPE
    if(control_conflict == true)
    {
        //printf("###fetch instruction bubbled ctrl\n");
    }
    #endif
    PC = PC_NEXT;//!!test version, should use later
    if(PC%4!=0)//iligal addr
    {
        printf("\n***invalid PC\n");
        return -1;
    }
    IR = *((INSTR*) (memory + PC));
    //!!need modify when bubble later
    PC_NEXT = PC+4;
    printf("###fetch instruction at 0x%08x\n",PC);
    return 0;
}
int decode_excute(INSTR inst)
{
    #ifndef SINGLE
    if(ctrl_BUBBLE_DI == true)
    {
        ctrl_BUBBLE_DI = false;
        ctrl_BUBBLE_WB = true;//!!import
        //printf("###decode&excute bubbled\n");
        return 0;//mean last instr is jump
    }
    #endif
    #ifdef PIPE
    if(control_conflict == true)
    {
        //printf("###decode&excute bubbled\n");
    }
    #endif
    printf("Decode&excute instruction: 0x%08x at 0x%x\n",inst,PC-4);
    INSTR opcode = get_opcode(inst);
    printf("\topcode: 0x%x",opcode);

    if(opcode == OPCODE_R)
    {
        int func3 = get_funct3(inst);
        int rd = get_rd(inst);
        int rs1 = get_rs1(inst);
        int rs2 = get_rs2(inst);
        int func7 = get_funct7(inst);
        if(func3 == 0x0)//add
        {
            if(func7 == 0x0)
            {
                int64_t ans = get_reg(rs1)+get_reg(rs2);
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = ans;
                printf("\tadd %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else if(func7 == 0x1)//mul
            {
                int64_t ans = get_reg(rs1)*get_reg(rs2);//lower 64 bit
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = ans;
                printf("\tmul %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else if(func7 == 0x20)//sub
            {
                int64_t ans = get_reg(rs1)-get_reg(rs2);
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = ans;
                printf("\tsub %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else
            {
                printf("\t!!!ERROR CODE in R,func3=0x%x,func7=0x%x\n",func3,func7);
                return -1;
            }            
        }
        else if(func3 == 0x1)
        {
            if(func7 == 0x0)//sll
            {
                int64_t ans = get_reg(rs1) << get_reg(rs2);
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = ans;
                printf("\tsll %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else if(func7 == 0x1)//mulh
            {
                int64_t val1 = get_reg(rs1);
                int64_t val2 = get_reg(rs2);                 
                int64_t ans = ((__int128_t)val1 * (__int128_t)val2) >> 64;
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = ans;
                printf("\tmulh %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else
            {
                printf("\t!!!ERROR CODE in R,func3=0x%x,func7=0x%x\n",func3,func7);
                return -1;
            }
        }
        else if(func3 == 0x2)
        {
            if(func7 == 0)//slt
            {
                int64_t ans = 0;
                if(get_reg(rs1) < get_reg(rs2))
                    ans = 1;
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = ans;
                printf("\tslt %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);  
            }
            else
            {
                printf("\t!!!ERROR CODE in R,func3=0x%x,func7=0x%x\n",func3,func7);
                return -1;
            }
        }       
        else if(func3 == 0x4)
        {
            if(func7 == 0x0)//xor
            {
                int64_t ans = get_reg(rs1) ^ get_reg(rs2);
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = ans;
                printf("\txor %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else if(func7 == 0x1)//div
            {
                int64_t ans = get_reg(rs1) / get_reg(rs2);
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = ans;
                printf("\tidv %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else
            {
                printf("\t!!!ERROR CODE in R,func3=0x%x,func7=0x%x\n",func3,func7);
                return -1;
            }
        }
        else if(func3 == 0x5)
        {
            if(func7 == 0x0)//srl
            {
                uint64_t ans = (uint64_t)(get_reg(rs1));//logic shift
                ans = ans >> get_reg(rs2);
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_MEM_val = (int64_t)ans;
                printf("\tsrl %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else if(func7 == 0x20)//sra
            {
                int64_t ans = get_reg(rs1);//albg shift
                ans = ans >> get_reg(rs2);
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_MEM_val = (int64_t)ans;
                printf("\tsra %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else
            {
                printf("\t!!!ERROR CODE in R,func3=0x%x,func7=0x%x\n",func3,func7);
                return -1;
            }
        }
        else if(func3 == 0x6)
        {
            if(func7 == 0x0)//or
            {
                int64_t ans = get_reg(rs1) | get_reg(rs2);
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = ans;
                printf("\tor %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else if(func7 == 0x1)//rem
            {
                int64_t ans = get_reg(rs1) % get_reg(rs2);
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = ans;
                printf("\trem %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else
            {
                printf("\t!!!ERROR CODE in R,func3=0x%x,func7=0x%x\n",func3,func7);
                return -1;
            }
        }
        else if(func3 == 0x7)
        {
            if(func7 == 0)
            {
                int64_t ans = get_reg(rs1) & get_reg(rs2);
                ctrl_wb_REG = true;
                wb_REG_No = rd;
                wb_REG_val = ans;
                printf("\tand %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            }
            else
            {
                printf("\t!!!ERROR CODE in R,func3=0x%x,func7=0x%x\n",func3,func7);
                return -1;
            }
        }
        else
        {
            printf("\t!!!ERROR CODE in R,func3=0x%x,func7=0x%x\n",func3,func7);
            return -1;
        }        
    }
    else if(opcode == OPCODE_I_1)//load memry
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
            printf("\taddi %s,%s,%ld,\n",regnames[rd],regnames[rs1],imm);
            //printf("\t %d,%d,%d shit",get_reg(rs1),imm,wb_REG_val);
        }
        else if(func3 == 0x1)//slli
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int64_t imm = get_imm_i(inst)&0x1f;//lower 5 bit
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = get_reg(rs1) << imm;
            printf("\tslli %s,%s,%ld\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x2)//slti
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int64_t imm = get_imm_i(inst);
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = (get_reg(rs1) < imm) ? 1 : 0;
            printf("\tslti %s,%s,%ld\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x4)//xori
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int64_t imm = (int64_t) (get_imm_i(inst));//sign extend
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = get_reg(rs1)^imm;
            printf("\txori %s,%s,%ld\n",regnames[rd],regnames[rs1],imm);
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
            printf("\tori %s,%s,%ld\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x7)//andi
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int64_t imm = (int64_t)(get_imm_i(inst));
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = get_reg(rs1) & imm;
            printf("\tandi %s,%s,%ld\n",regnames[rd],regnames[rs1],imm);
        }
    }//endif op 0x13
    else if(opcode == OPCODE_I_3)
    {
        int func3 = get_funct3(inst);
        int func7 = get_funct7(inst);
        printf("!!!!!shit %x,%x",func3,func7);
        if(func3 == 0x0)//addiw
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int64_t imm = (int64_t)(get_imm_i(inst));
            int64_t ans = get_reg(rs1) + imm;
            int32_t tmp = ans;
            ans = tmp;//!!sign extend
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = ans;
            printf("\taddiw %s,%s,%ld\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x5 && (func7>>1) == 0x10)//sraiw
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int imm = (int)((get_imm_i(inst)) & 0x1f);//5 bits

            ctrl_wb_REG = true;
            wb_REG_No = rd;
            int32_t tmp = (int32_t)(get_reg(rs1));//alg shift in32 bit
            tmp = tmp >> imm;

            wb_REG_val = (int64_t)tmp;//sign extend
            printf("\tsraiw %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else if(func3 == 0x5 && (func7>>1) == 0x0)//srliw
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int imm = (int)((get_imm_i(inst)) & 0x1f);;

            ctrl_wb_REG = true;
            wb_REG_No = rd;
            uint32_t tmp = (uint32_t)(get_reg(rs1));//logic shift

            tmp = tmp >> imm;
            wb_REG_val = (int64_t)tmp;//signextend
            printf("\tsrliw %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else
        {
            printf("\t!!!ERROR CODE in opcode_i_3\n");
            return -1;
        } 
    }
    else if(opcode == OPCODE_I_4)//jalr
    {
        int func3 = get_funct3(inst);
        if(func3 == 0x0)
        {
            int rd = get_rd(inst);
            int rs1 = get_rs1(inst);
            int imm = get_imm_i(inst);
            //set PC
            PC_NEXT = (get_reg(rs1) + imm)&0xfffffffe;
            //set rd
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            //wb_REG_val = PC;//!!!when decode PC point to next instrct
            wb_REG_val = PC + 4;//!!!when decode PC point to next instrct
            //bubble
            //ctrl_BUBBLE_DI = true;
            //ctrl_BUBBLE_WB = true;
            printf("\tjalr %s,%s,%d\n",regnames[rd],regnames[rs1],imm);
        }
        else
        {
            printf("\t!!!ERROR CODE in opcode_i_4\n");
            return -1;
        }
        
    }
    else if(opcode == OPCODE_I_5)
    {
        int func3 = get_funct3(inst);
        int func7 = get_funct7(inst);
        if(func3 == 0x0 && func7 == 0x0)
        {
            int ra0 = get_reg(10);
            printf("\tecall a0=%d\n",ra0);
            syscall(ra0);
        }
        else
        {
            printf("\t!!!ERROR CODE in opcode_i_5\n");
            return -1;
        }        
    }
    
    else if(opcode == OPCODE_U_1)//auipc
    {
        int rd = get_rd(inst);
        int64_t off = get_imm_u(inst);
        ctrl_wb_REG = true;
        wb_REG_No = rd;
        //wb_REG_val = (int64_t)PC - 4 + off;//its not right,next PC is =4? no jump
        wb_REG_val = (int64_t)PC  + off;//its not right,next PC is =4? no jump
        printf("\tauipc %s,0x%lx\n",regnames[rd],off>>12);
    }
    else if(opcode ==  OPCODE_U_2)//lui
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
        //PC_NEXT = PC - 4 + imm;//!!!when decode PC point to next instrct
        PC_NEXT = PC + imm;
        //set rd
        ctrl_wb_REG = true;
        wb_REG_No = rd;
        //wb_REG_val = PC;//!!!when decode PC point to next instrct
        wb_REG_val = PC + 4;
        //bubble
        //ctrl_BUBBLE_DI = true;
        //ctrl_BUBBLE_WB = true;
        printf("\tjal %s,0x%x:0x%x\n",regnames[rd],imm,PC_NEXT);//!!not val in dump
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
            printf("\tsb %s,%d(%s):0x%lx\n",regnames[rs2],off,regnames[rs1],addr);
        }
        if(func3 == 1)//sh
        {
            ctrl_wb_MEM = true;
            wb_MEM_off = addr;
            wb_MEM_len = 2;
            wb_MEM_val = (uint64_t)(get_reg(rs2)&0xffff);
            printf("\tsh %s,%d(%s):0x%lx\n",regnames[rs2],off,regnames[rs1],addr);
        }
        if(func3 == 2)//sw
        {
            ctrl_wb_MEM = true;
            wb_MEM_off = addr;
            wb_MEM_len = 4;
            wb_MEM_val = (uint64_t)(get_reg(rs2)&0xffffffff);
            printf("\tsw %s,%d(%s):0x%lx\n",regnames[rs2],off,regnames[rs1],addr);
            //printf("%d,%d,%d",get_reg(rs2),get_reg(rs1), off);
        }
        if(func3 == 3)//sd
        {
            ctrl_wb_MEM = true;
            wb_MEM_off = addr;
            wb_MEM_len = 8;
            wb_MEM_val = (uint64_t)get_reg(rs2);
            printf("\tsd %s,%d(%s):0x%lx\n",regnames[rs2],off,regnames[rs1],addr);
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
                PC_NEXT = off + PC;
            printf("\tbeq %s,%s,%d at 0x%d\n",regnames[rs1],regnames[rs2],off,off + PC - 4);
        }
        else if(func3 == 0X1)//bne
        {
            if(get_reg(rs1) != get_reg(rs2))
                PC_NEXT = off + PC;
            printf("\tbne %s,%s,%d at 0x%d\n",regnames[rs1],regnames[rs2],off,off + PC - 4);
        }
        else if(func3 == 0X4)//blt
        {
            if(get_reg(rs2) < get_reg(rs2))
                PC_NEXT = off + PC;
            printf("\tblt %s,%s,%d at 0x%d\n",regnames[rs1],regnames[rs2],off, off + PC - 4);
        }
        else if(func3 == 0X5)//bge
        {
            if(get_reg(rs1) >= get_reg(rs2))
                PC_NEXT = off + PC;
            printf("\tbge %s,%s,%d at 0x%d\n",regnames[rs1],regnames[rs2],off,off + PC - 4);
        }
        else if(func3 == 0X6)//bltu
        {
            if((uint64_t)(get_reg(rs1)) < (uint64_t)(get_reg(rs2)))
                PC_NEXT = off + PC;
            printf("\tbltu %s,%s,%d at 0x%d\n",regnames[rs1],regnames[rs2],off,off + PC - 4);
        }
        else if(func3 == 0X7)//bgeu
        {
            if((uint64_t)(get_reg(rs1)) >= (uint64_t)(get_reg(rs2)))
                PC_NEXT = off + PC;
            printf("\tbltu %s,%s,%d at 0x%d\n",regnames[rs1],regnames[rs2],off,off + PC - 4);
        }
        else
        {
            printf("\t!!!ERROR CODE in opcode_sb,func3=0x%x\n",func3);
            return -1;
        }
    }
    else if(opcode == OPCODE_W)
    {
        int func3 = get_funct3(inst);
        int func7 = get_funct7(inst);
        int rs1 = get_rs1(inst);
        int rs2 = get_rs2(inst);
        int rd = get_rd(inst);
        if(func3 == 0 && func7 == 0)//addw
        {
            int64_t ans = get_reg(rs1)+get_reg(rs2);
            int32_t tmp = ans;//32bit
            ans = tmp;//sign extend
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = ans;
            printf("\taddw %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]);
            
        }
        else if(func3 == 0 && func7 == 0x1)//mulw
        {
            int64_t ans = get_reg(rs1) * get_reg(rs2);
            int32_t tmp = ans;//32bit
            ans = tmp;//sign extend
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = ans;
            printf("\tmulw %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]); 
        }
        else if(func3 == 0 && func7 == 0x20)//subw
        {
            int64_t ans = get_reg(rs1) - get_reg(rs2);
            int32_t tmp = ans;//32bit
            ans = tmp;//sign extend
            ctrl_wb_REG = true;
            wb_REG_No = rd;
            wb_REG_val = ans;
            printf("\tmultw %s,%s,%s\n",regnames[rd],regnames[rs1],regnames[rs2]); 
        }
        else
        {
            printf("\t!!!ERROR CODE in opcode_W,func3=0x%x,func7=0x%x\n",func3,func7);
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
    #ifdef PIPE
    if(data_conflict == true)
    {
        //printf("###write back bubble! data\n");
    }
    #endif
    #ifndef SINGLE
    if(ctrl_BUBBLE_WB == true)
    {
        //printf("###write back bubble!\n");
        ctrl_BUBBLE_WB = false;
        return 0;
    }
    #endif
    if(ctrl_wb_MEM == true)
    {
        printf("###write back memory off = %d,val = %ld, len = %d\n",wb_MEM_off,wb_MEM_val,wb_MEM_len);
        if(wb_MEM_off > MEM_SIZE)
        {
            printf("***write back false: offset out of size!\n");
            return -2;
        }   
        if(wb_MEM_len == 1)//sb
        {
            int8_t tmp = wb_MEM_val;
            int8_t* into = (int8_t *) &memory[wb_MEM_off];
            *into = tmp;
        }
        else if(wb_MEM_len == 2)//sh
        {
            int16_t tmp = wb_MEM_val;
            int16_t* into = (int16_t *) &memory[wb_MEM_off];
            *into = tmp;
        }
        else if(wb_MEM_len == 4)//sw
        {
            int32_t tmp = wb_MEM_val;
            int32_t* into = (int32_t *) &memory[wb_MEM_off];
            *into = tmp;
        }
        else if(wb_MEM_len == 8)//sd
        {
            int64_t tmp = wb_MEM_val;
            int64_t* into = (int64_t *) &memory[wb_MEM_off];
            *into = tmp;
        }
        else
        {
            printf("***write back false: length iligal!\n");
            return -1;
        }
        
    }
    if(ctrl_wb_REG == true)
    {
        if(wb_REG_No == 0)
            return 0;
        if(wb_REG_No > REG_NUM)
        {
            printf("***write back false: REG No %d doesnt exist!\n",wb_REG_No);
            return -1;
        }
        printf("###write back register no = %d,val = %ld = %lx\n",wb_REG_No,wb_REG_val,wb_REG_val);
        regs[wb_REG_No] = wb_REG_val;
    }
    return 0;
}
void print_all_REGS()
{
    printf("REG values:\n");
    for (int i = 0; i < REG_NUM; i++)
    {
       printf("%s %d:0x%lx = %ld\n",regnames[i],i,regs[i],regs[i]);
    }    
}
int print_memory(int off,int len)
{
    if(wb_MEM_off > MEM_SIZE)
    {
    printf("***read memory false: offset out of size!\n");
        return -2;
    }   
    int val;
    printf("Memory content:\n");
    if(len == 1)//sb
    {
        int8_t* into = (int8_t *) &memory[off];
        val = *into;
        printf("%02x\n",val);
    }
    else if(len == 2)//sh
    {
        int16_t* into = (int16_t *) &memory[off];
        val = *into;
        printf("%04x\n",val);
    }
    else if(len == 4)//sw
    {
        int32_t* into = (int32_t *) &memory[off];
        val = *into;
        printf("%08x\n",val);
    }
    else if(len == 8)//sd
    {
        int64_t* into = (int64_t *) &memory[off];
        val = *into;
        printf("%016x\n",val);
    }
    else
    {
        printf("***fead memory false: length iligal!\n");
        return -1;
    }
    return 0;
}