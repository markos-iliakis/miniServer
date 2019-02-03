#define _GNU_SOURCE
#include "queue.h"
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <dirent.h>

/* Libraries included in my headers */
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

extern pthread_mutex_t ac;
extern sem_t sem;
extern char** visited;
extern int count, pages;
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
    int port;
    char* ip;
}args;

thr_pool* thr_pool_Init(int num, Queue* queue, char* new_dir, int s_port, char* ip);
args* args_Init(Queue* queue, char* new_dir, int s_port, char* ip);

void my_write(char* page, int fd, char* head);
void my_read(char** buf, int fd);
int my_connect(int s_port, char* ip);

void* request(void* queue);

char* forge_request(char* url);
char** extract_links(char* buf, int* link_num, char* ip, int port);
void create_file(char* str1, char* str2, char* buf);
int link_nvisited(char* link);

int sock_Init(int port, struct sockaddr_in* server, char* str_ip, const char* type);
int isIp_v4( char* ip);
int hostname_to_ip(char* hostname, in_addr_t* addr);
int ip_to_ip(char* str_ip, in_addr_t* addr);
void to_ip(struct sockaddr_in* server, char* str, in_addr_t* addr);

void perror2(const char* s, int err);
void thr_restart(Queue* queue, thr_pool* pool, int i, char* new_dir, int s_port, char* ip);