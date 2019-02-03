#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <ctype.h>
#include <dirent.h>
#include <time.h>
#include <sys/ioctl.h>
#include "trie.h"

#define MAX_Q_NUM 10

char** parseFile(char* filename, int* counter);
void createIndexes(char** text, Trie* trie, int text_num, char* filename, int file_index);
void send_searchAnswer(char* path, int line_num, char* line, int fd_pipe);
void search(char** qs, Trie* root, char*** text, int fd_pipe, int log_fd, char* t);
int numbers_only(const char* s);
int make_fifos(char** name_r, char** name_w, int* fd_r, int* fd_w);
int read_paths(int fd, char** paths, int n);
int read_files(int n, char** paths, char**** text, int** txt_num, Trie* trie, int* lines, int* file_cnt);
int read_command(char** buffer, int* command, int fd, char* name);
void seperate_words(char*** qs, char* buffer);
void send_count_answer(int found, Plist* lroot, int fd_w, int fd_l, char* name, time_t rawtime, char* token, int flag);
int my_read(int fd, char** buffer, int size);
void perror_exit(char* message);
