#include"naive_sim.h"
/***exculte part***/
int fetch_instr(unsigned int pc)//
{
    if(pc%4!=0)
    {
        printf("invalid PC");
        return -1;
    }
    
}

int run_one_instr(INSTR x)
{
    int opcode = get_opcode(x);
    switch(opcode)
    {
        default: break
    }
}