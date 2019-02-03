#include "workerfunc.h"

int main(int argc, char const *argv[]) {
	// printf("Worker: worker made\n");
	Trie* trie = TrieInit();
	pid_t pid = getpid();
	char* pipe_name_r = calloc(20, sizeof(char));
	char* pipe_name_w = calloc(20, sizeof(char));
	int fd_pipe_read, fd_pipe_write, log_fd;
	int n = atoi(argv[1]);
	char*** text = malloc(n * sizeof(char**));
	int file_cnt = 0, max_files = n;
	int lines = 0;
	time_t rawtime;
	struct tm * timeinfo;
	int* txt_num;
	int index = 0;
	int s = 1;
	int c;

	char** paths = malloc(n*sizeof(char*));
	for (size_t i = 0; i < n; i++) {
		paths[i] = calloc(SIZE_OF_ADDRESS, sizeof(char));
	}

	// txt_num = malloc(s * sizeof(int));
	// txt_num[0] = 0;



	// open log file
	// printf("Worker : making log file\n");
	char log_name[15];
	sprintf(log_name, "./log/Worker_%d", pid);
	if((log_fd = open(log_name, O_WRONLY | O_APPEND | O_CREAT, 0666)) < 0){
		perror("Worker : log file open");
	}

	// open the pipes(read-write)
	// printf("Worker : make fifos\n");
	make_fifos(&pipe_name_r, &pipe_name_w, &fd_pipe_read, &fd_pipe_write);

	// read the paths once from pipe
	// printf("about to read %d paths\n", n);
	read_paths(fd_pipe_read, paths, n);

	// read the files
	// printf("Worker : about to read files\n");
	read_files(n, paths, &text, &txt_num, trie, &lines, &file_cnt);

	int command;
	char* buffer;
	// printf("Worker: about to read command\n");
	// read command through pipe
	int size = read_command(&buffer, &command, fd_pipe_read, pipe_name_r);

	// execute//search
	// printf("Worker : search command\n");
	char** qs = NULL;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	// seperate the words to search
	seperate_words(&qs, buffer);

	char t[20];
	sprintf(t, "%ld", rawtime);
	t[strlen(t)-1] = '\0';

	// search, log and send back the results
	search(qs, trie, text, fd_pipe_write, log_fd, t);

	for (size_t i = 0; i < 10; i++) {
		if(qs[i]!=NULL){
			free(qs[i]);
			qs[i] = NULL;
		}
	}
	free(qs);
	qs = NULL;
	free(buffer);
	buffer = NULL;
	// printf("Worker %d : finished\n", pid);
		
	close(log_fd);

	for (size_t i = 0; i < file_cnt; i++) {
		for (size_t j = 0; j < (txt_num)[i]; j++) {
			free(text[i][j]);
		}
		free(text[i]);
	}
	free(text);

	free(txt_num);
	free(pipe_name_w);
	free(pipe_name_r);

	for (size_t i = 0; i < n; i++) {
		free(paths[i]);
	}
	free(paths);
	TrieDestroy(trie->head);
	return 0;
}
