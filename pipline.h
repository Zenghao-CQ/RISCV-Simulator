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
extern int wb_MEM_len;

extern bool ctrl_read_REG;
extern bool ctrl_read_MEM;
extern int read_REG_No;
extern int read_MEM_off;
extern int read_MEM_len;
#ifdef PIPE
extern bool data_conflict;
extern bool control_conflict;
#endif
int64_t wb_REG_val;
uint64_t wb_MEM_val;//no sign extend

/***intitial state***/
extern void init();
/***laod to memory from file***/
extern int load_memory(char* filname);
extern void print_all_REGS();
extern int print_memory(int off,int len);
/***exculte part***/

extern int fetch_instr();//into IR
extern int decode_excute(INSTR inst);//frome IR
extern int write_back();
#endif //PIPLINE