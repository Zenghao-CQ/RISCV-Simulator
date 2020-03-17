# RISCV simulator
>Lab for  *"Computer Organization and Achitecture"* course of **PKU**  \
>Support **RV64I** instruction set with a **3 stage pipline** implemented by using **C**  \
>Temporarily the  project do not support interrupt including **syscal** and **Link library**

>There 2 modes you can choose:
>>1. **Single Step**:  set ```CFLAGS = -DSINGLE``` in Makefile and make. the commands are as follow:
>>'c'(continue) run one instruction  \
>>'r'(rigisters) print all rigisters  \
>>'g'(go) keep running untill end of "main" section  \
>>'m'(memory) print memory. Type as fomat "offset width", width will be ilegal except 1,2,4,8 Bytes
>>2. **Pipline**: set ```CFLAGS = ``` and make

>To run this project, you need:\
>```make```\
>```./riscv filename```