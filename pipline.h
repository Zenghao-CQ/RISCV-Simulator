#ifndef PIPLINE
#define PIPLINE
#include"cpu.h"
#include"elf.h"


/***hard ware***/
extern unsigned int PC;
extern unsigned int PC_NEXT;
extern INSTR IR;
/***laod to memory from file***/
extern int load_memory(char* filname);

/***exculte part***/

extern int fetch_instr(int pc);//into IR
extern int decode_excute();//frome IR

#endif //PIPLINE