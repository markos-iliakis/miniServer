#include "threadpool.h"
#include "common.h"
#include <signal.h>
#include <errno.h>
#include <sys/wait.h> /* sockets */
#include <sys/types.h> /* sockets */
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/time.h>

/* Libraries included in my headers */
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/fcntl.h>
// #include <sys/stat.h>
// #include <string.h>
// #include <semaphore.h>
// #include <sys/socket.h> /* sockets */
// #include <netinet/in.h> /* internet sockets */
// #include <unistd.h> /* fork */
// #include <sys/ioctl.h>
// #include <pthread.h>
// #include <dirent.h>
// #include <netdb.h>

#define DEADLINE 10
#define W 1

extern int path_num;

char* parseArgs(int argc, char* argv[], int* s_port, int* c_port, int* tr_num, char** ip, char** start_url);

void command_handler(int sock_c, struct timeval* start_time, Queue* queue, char* new_dir);
void my_read2(char** buf, int fd);
char** extract_paths(char* new_dir);
int make_worker(int n);
int make_fifos(int pid, char** name_r, char** name_w, int* fd_r, int* fd_w);
int share_paths(int fd, char** paths, char* name, int* k, int n);
int send_command(int command, int len, int fd, char* buffer, char* name);
void jobExecutor(char* new_dir, int sock_c, char* buffer);
void poll_Init(struct pollfd* fdr, int* fd);
int check_time(int pid, int start_time, int deadline);

void perror_exit(char* message);