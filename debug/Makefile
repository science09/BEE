TARGET = dbg.o
CC=g++
CFLAGS = -c -g -Wall -DDBG_NO_DEBUG -ansi -pedantic -std=c++11
OBJS = debug.o
INCLUDES = -I..

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $*.c
debug.o: debug.c ../MEM.h debug.h ../DBG.h
