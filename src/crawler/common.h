#include <unistd.h>
#include <errno.h>

#define SIZE_OF_ADDRESS 100

typedef struct answer{
	char path[SIZE_OF_ADDRESS];
	int line_num;
	char line[1024];
}answer;

typedef struct wc{
	long int letters;
	int words;
	int lines;
}wc;

int make_fifo_name(pid_t pid, char* name, int flag);
int mywrite(int fd, char *buff, size_t size);
char* tokenize(char** buffer, char del);
