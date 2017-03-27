TARGET = bee
CC=gcc
OBJS = \
  lex.yy.o\
  y.tab.o\
  main.o\
  interface.o\
  create.o\
  execute.o\
  eval.o\
  string.o\
  string_pool.o\
  util.o\
  builtin.o\
  error_message.o\
  ./memory/mem.o\
  ./debug/dbg.o
CFLAGS = -c -g -Wall -Wswitch-enum -ansi -pedantic -DDEBUG -std=c99
INCLUDES = \

$(TARGET):$(OBJS)
	cd ./memory; $(MAKE);
	cd ./debug; $(MAKE);
	$(CC) $(OBJS) -o $@ -lm -pthread
clean:
	rm -f *.o lex.yy.c y.tab.c y.tab.h *~
y.tab.h : bee.y
	bison --yacc -dv bee.y
y.tab.c : bee.y
	bison --yacc -dv bee.y
lex.yy.c : bee.l bee.y y.tab.h
	flex bee.l
y.tab.o: y.tab.c bee_def.h MEM.h
	$(CC) -c -g $*.c $(INCLUDES)
lex.yy.o: lex.yy.c bee_def.h MEM.h
	$(CC) -c -g $*.c $(INCLUDES)
.c.o:
	$(CC) $(CFLAGS) $*.c $(INCLUDES)
./memory/mem.o:
	cd ./memory; $(MAKE);
./debug/dbg.o:
	cd ./debug; $(MAKE);
############################################################
create.o: create.c MEM.h DBG.h bee_def.h BEE.h BEE_dev.h
#error.o: error.c MEM.h bee_def.h BEE.h BEE_dev.h
error_message.o: error_message.c bee_def.h MEM.h BEE.h BEE_dev.h
eval.o: eval.c MEM.h DBG.h bee_def.h BEE.h BEE_dev.h
execute.o: execute.c MEM.h DBG.h bee_def.h BEE.h BEE_dev.h
interface.o: interface.c MEM.h DBG.h bee_def.h BEE.h BEE_dev.h
main.o: main.c BEE.h MEM.h
builtin.o: builtin.c MEM.h DBG.h bee_def.h BEE.h BEE_dev.h
string.o: string.c MEM.h bee_def.h BEE.h BEE_dev.h
string_pool.o: string_pool.c MEM.h DBG.h bee_def.h BEE.h BEE_dev.h
util.o: util.c MEM.h DBG.h bee_def.h BEE.h BEE_dev.h
