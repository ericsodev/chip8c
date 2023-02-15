all: chip.o main

main: chip.o chip.h
	gcc -I src/include -L src/lib -o main main.c chip.o -lmingw32 -lSDL2main -lSDL2


chip.o: chip.c chip.h
	gcc -g -c chip.c