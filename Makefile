riscv: main.o cpu.o elf.o pipline.o
	cc -o riscv cpu.o elf.o pipline.o main.o

main.o: main.c pipline.h elf.h cpu.h
	cc -c main.c

pipline.o: pipline.c pipline.h elf.h cpu.h
	cc -c pipline.c

cpu.o: cpu.c cpu.h
	cc -c cpu.c

elf.o: elf.c elf.h
	cc -c elf.c

clean:
	rm *.o riscv
	