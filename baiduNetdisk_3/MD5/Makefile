SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
ELF:=client
CC:=gcc  #编译器
CFLAGS:=-g -Wall  #生成时候需要加的标志
$(ELF):$(OBJS)
	gcc -o $@ $^ -lcrypt
clean:
	rm -rf $(OBJS) $(ELF)
