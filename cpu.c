#include "cpu.h"

/***data struct***/
//registers
int64_t regs[REG_NUM];
//debug 
bool bug_flag;
char regnames[32][5] = {"zero","ra","sp","gp","tp","t0","t1","t2",
						"s0","s1","a0","a1","a2","a3","a4","a5",
						"a6","a7","s2","s3","s4","s5","s6","s7",
						"s8","s9","s10","s11","t3","t4","t5","t6"};

//memorys
unsigned char memory[MEM_SIZE];

/***instruction decode***/
INSTR get_imm_s(INSTR x)
{
    INSTR low = get_rd(x);
    INSTR hig = (x >> 25) << 5; //with sign
    return (INSTR)(hig | low);
}

INSTR get_imm_sb(INSTR x)
{
    INSTR imm12 =  x >> 31;//with sign
	INSTR imm11 =  (x >> 7) & 0x1;
	INSTR imm12_11 = (imm12 << 1) | imm11;
	INSTR imm10_5 = (x >> 25) & 0x03F;
	INSTR imm12_5= (imm12_11 << 6) | imm10_5;
	INSTR imm4_1=(x >> 8) & 0x0F;
	INSTR imm12_1=(imm12_5 << 4) | imm4_1;
	INSTR imm12_0=imm12_1 << 1;
	return imm12_0;
}

// INSTR get_imm_sb(INSTR x)
// {
//     INSTR imm4_1 = (x >> 8) & 0xf;
//     imm4_1 = imm4_1 << 1;
//     INSTR imm10_5 = (x >> 25) & 0x03F;
//     imm10_5 = imm10_5 << 5;
//     INSTR imm11 = (x >> 7) & 0x1;
//     imm11 = imm11 << 11;
//     INSTR imm12 = x >> 31; // exten with sign
//     imm12 = imm12 << 12;
//     return (imm4_1 | imm10_5 | imm11 | imm12);
// }

INSTR get_imm_uj(INSTR x)
{
    INSTR imm20 = x >> 31;//with sign
    INSTR imm19_12 = (x >> 12) & 0x0FF;
    INSTR imm20_12 = (imm20 << 8) | imm19_12;
    INSTR imm11 = (x >> 20) & 0x1;
    INSTR imm20_11 = (imm20_12 << 1) | imm11;
    INSTR imm10_1 = (x >> 21) & 0x03FF;
    INSTR imm20_1 = (imm20_11 << 10) | imm10_1;
    INSTR imm20_0 = imm20_1 << 1;
    return imm20_0;
}
void DEBUG(const char* str)
{
	if(bug_flag == true)
	{
		printf("***DEBUG: %s\n",str);
	}
}