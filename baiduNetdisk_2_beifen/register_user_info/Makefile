SRCS:=$(wildcard *.c)
ELFS:=$(SRCS:%.c=%)
CC:=gcc
all:$(ELFS)

%:%.c
	$(CC) $< -o $@ -lcrypt -lmysqlclient
clean:
	rm -rf $(ELFS)
