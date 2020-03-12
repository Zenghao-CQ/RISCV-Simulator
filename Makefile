riscv: main.o cpu.o
	cc -o riscv main.o cpu.o;rm *.o
main.o: main.c cpu.h 
	cc -c main.c
cpu.o: cpu.c cpu.h
	cc -c cpu.c

clean:
	rm *.o riscv
	