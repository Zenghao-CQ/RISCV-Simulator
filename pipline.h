#ifndef PIPLINE
#define PIPLINE
#include"cpu.h"
#include"elf.h"


/***hard ware***/
extern unsigned int PC;
extern unsigned int END;
extern unsigned int PC_NEXT;
extern INSTR IR;
/***contrl***/
extern bool ctrl_wb_REG;//if write back reg file
extern bool ctrl_wb_MEM;//if write back memory
extern bool ctrl_BUBBLE_DI;//if branch predict is wrong, in di
extern bool ctrl_BUBBLE_WB;//if branch predict is wrong, in wb
extern int wb_REG_No;
extern int wb_MEM_off;
REG wb_REG_val;
REG wb_MEM_val;

/***intitial state***/
extern void inti();
/***laod to memory from file***/
extern int load_memory(char* filname);

/***exculte part***/

extern int fetch_instr(int PC);//into IR
extern int decode_excute(INSTR inst);//frome IR
extern int write_back();
#endif //PIPLINE