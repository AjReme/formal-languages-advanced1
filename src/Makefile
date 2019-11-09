all: main

main: lib.o main.o
	arm-linux-gnueabi-g++ lib.o main.o -o main

sol.o: lib.cpp
	arm-linux-gnueabi-g++ -marm lib.cpp -c

main.o: main.c
	arm-linux-gnueabi-gcc main.c -c -marm

clean:
	rm lib.o main.o
