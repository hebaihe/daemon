SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
ELF:=server
CC:=gcc
CFLAGS:=-g  -I /usr/include/mysql/  -Wall 
$(ELF):$(OBJS)
	gcc -o $@ $^ -lcrypt -lmysqlclient
clean:
	rm -rf $(OBJS) $(ELF)
