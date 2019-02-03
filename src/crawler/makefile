OBJS1 = crawler.o crawlerfunc.o threadpool.o queue.o common.o
OBJS2 = worker.o workerfunc.o common.o trie.o
SOURCE1 = crawler.c crawlerfunc.c threadpool.c queue.c common.c
SOURCE2 = worker.c workerfunc.c common.c trie.c
HEADER1 = crawler.h threadpool.h queue.h common.h
HEADER2 = workerfunc.h common.h trie.h
OUT1 = mycrawler
OUT2 = worker
CC = gcc
FLAGS = -g -c

all : $(OUT1) $(OUT2)

$(OUT1) : $(OBJS1)
	$(CC) -g $(OBJS1) -o $@ -lm -lpthread

$(OUT2) : $(OBJS2)
	$(CC) -g $(OBJS2) -o $@ -lm

clean:
	rm -r $(OUT1) $(OBJS1) $(OUT2) worker.o workerfunc.o trie.o
