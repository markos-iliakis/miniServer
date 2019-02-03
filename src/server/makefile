OBJS1 = server.o serverfunc.o threadpool.o queue.o
SOURCE1 = server.c serverfunc.c threadpool.c queue.c
HEADER1 = server.h threadpool.h queue.h
OUT1 = myhttpd
CC = gcc
FLAGS = -g -c

$(OUT1) : $(OBJS1)
	$(CC) -g $(OBJS1) -o $@ -lm -lpthread

clean:
	rm -r $(OUT1) $(OBJS1)
