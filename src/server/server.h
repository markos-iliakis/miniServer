#include "threadpool.h"
#include <time.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/wait.h> /* sockets */
#include <sys/types.h> /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <arpa/inet.h>
#include <netdb.h> /* ge th os tb ya dd r */
#include <ctype.h> /* toupper */
#include <dirent.h>
#include <math.h>
#include <sys/time.h>

/* Libraries included in my headers */
// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <string.h>
// #include <semaphore.h>
// #include <unistd.h>
// #include <errno.h>

char* parseArgs(int argc, char* argv[], int* s_port, int* c_port, int* tr_num);
void perror_exit(char* message);
void my_read(char** buf, int fd);
off_t fsize(char *filename);
void poll_Init(struct pollfd* fdr, int fd, int fd2);
void GET_handler(int sock, char* root_dir, int* pages, long long int* bytes, Queue* queue);
int command_handler(int sock_c, struct timeval* start_time, int pages, long long int bytes);
int sock_Init(int port, struct sockaddr_in* server);
int valid(char* req);