riscv: main.o cpu.o elf.o pipline.o syscall.o
	cc -o riscv cpu.o elf.o pipline.o main.o syscall.o

syscall.o: syscall.c syscall.h cpu.h
	cc -c syscall.c

main.o: main.c pipline.h elf.h cpu.h syscall.h 
	cc -c main.c

pipline.o: pipline.c pipline.h elf.h cpu.h syscall.h
	cc -c pipline.c

cpu.o: cpu.c cpu.h
	cc -c cpu.c

elf.o: elf.c elf.h
	cc -c elf.c

clean:
	rm *.o riscv
	