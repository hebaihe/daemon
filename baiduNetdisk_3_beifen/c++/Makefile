SRCS:=$(wildcard *.cc)
CC:=gcc
CFLAGS:=-g -Wall # -lpthread
server:$(SRCS)
	$(CC) -o $@ $^ $(CFLAGS)
.PHONY: clean
clean:
	rm $(OBJS) server
