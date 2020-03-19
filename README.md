# RISCV simulator
>Lab for  *"Computer Organization and Achitecture"* course of **PKU**  \
>Support **RV64I** instruction set with a **3 stage pipline** implemented by using **C**  \
>Temporarily the  project do not support interrupt including **syscal** and **Link library**  \
>The ELF part use opensource code at https://github.com/macmade/ELFDump/tree/master/ELFDump

>There 2 modes you can choose:
>>1. **Single Step**:  set ```CFLAG = -DSINGLE``` in Makefile and make. the commands are as follow:  \
>>'c'(continue) run one instruction  \
>>'r'(rigisters) print all rigisters  \
>>'g'(go) keep running untill end of "main" section  \
>>'m'(memory) print memory. Type as fomat "offset width", width will be ilegal except 1,2,4,8 Bytes
>
>>2. **Pipeline**: set ```CFLAG = -DPIPE``` and make

>The RISCV program is compiled by [RISC-V Tools](https://github.com/riscv/riscv-tools), you can use following command to compile: \
>```riscv64-unkown-elf-gcc -Wa,-march=rv64i -o filename filename.c```
>or
>```riscv64-unkown-elf-gcc -Wa,-march=rv64g -o filename filename.c``` (for rv64M instructions)

>To run this project, you need:\
>```make```\
>```./riscv filename```
