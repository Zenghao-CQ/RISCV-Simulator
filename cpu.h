#ifndef CPU
#define CPU

#include<stdio.h>
#include<stdlib.h>

/***data struct***/
//OPcdoe
#define OPCODE_R 0x33 //R
#define OPCODE_I_1 0x03 //I
#define OPCODE_I_2 0x13
#define OPCODE_I_3 0x1B
#define OPCODE_I_4 0x67
#define OPCODE_I_5 0x73
#define OPCODE_S 0x23 //S
#define OPCODE_SB 0x63 //SB
#define OPCODE_U_1 0x17 //U
#define OPCODE_U_2 0x37
#define OPCODE_UJ 0x6f //UJ

//register
#define REG_NUM 32 
typedef __int32_t INSTR;//!!imm need sign extend
typedef __uint64_t REG;
extern REG regs[REG_NUM];//all register but xo
char regnames[32][5] = {"zero","ra","sp","gp","tp","t0","t1","t2",
						"s0","s1","a0","a1","a2","a3","a4","a5",
						"a6","a7","s2","s3","s4","s5","s6","s7",
						"s8","s9","s10","s11","t3","t4","t5","t6"};
//memory
#define MEM_SIZE 1024*1024//1M
extern unsigned char memory[MEM_SIZE];

/***instruction decode***/
#define get_opcode(x) (x & 0x7f)
#define get_rd(x) ((x>>7) & 0x1f) //0xf80
#define get_funct3(x) ((x>>12) & 0x7) //0x7000
#define get_rs1(x) ((x>>15) & 0x1f) //0xf8000
#define get_rs2(x) ((x>>20) & 0x1f) //0x1f00000
#define get_funct7(x) ((x>>25) & 0x7f) //0xfe000000
//decode get imm number
#define get_imm_i(x) (x>>20) //sifn extend
extern INSTR get_imm_s(INSTR x); //sifn extend
extern INSTR get_imm_sb(INSTR x); //sifn extend
#define get_imm_u(x) ((x>>12)<<12) 
extern INSTR get_imm_uj(INSTR x);

//hard ware
extern unsigned int PC;
extern unsigned int PC_NEXT;
extern INSTR IR;

#endif //CPU