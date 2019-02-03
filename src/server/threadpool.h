#define _GNU_SOURCE
#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>

/* Libraries included in my headers */
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>


extern pthread_mutex_t ac;
extern sem_t sem;
extern int pages;
extern long long int bytes;
extern int flag;
int flag2;

typedef struct thr_pool{
    pthread_t* thr;
    int thr_num; 
}thr_pool;

typedef struct args{
    Queue* queue;
    char* dir;
}args;

thr_pool* thr_pool_Init(int num, Queue* queue, char* root_dir);
void my_write(char* page, int fd, char* head);
void* response(void* queue);
void perror2(const char* s, int err);
void thr_restart(Queue* queue, thr_pool* pool, int i);
int get_page(char** page, char* root_dir, char* buf);
args* args_Init(Queue* queue, char* new_dir);