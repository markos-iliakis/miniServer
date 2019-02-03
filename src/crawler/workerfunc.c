#include "workerfunc.h"

char** parseFile(char* filename, int* counter){

    size_t maxl = 1024;
	char* l = malloc(maxl * sizeof(char));
    char* line = l;
    char** text = NULL;
    int length;
    int ptr = 0;
	// printf("filename : %s\n", filename);
	FILE* file = fopen(filename, "r");

    while(fgets(line, maxl, file) != NULL){
		if(strcmp(line, "</html>\n") == 0) break;
		// if(line[0] == '<' || line[0] == '\n') continue;
		
		// printf("reading from file :  %s\n", line);
        // check if we have space for the hole line
        while(line[strlen(line)-1] != '\n' && line[strlen(line) -1] != '\r'){
            l = realloc(l, 2*maxl*sizeof(char));
			line = l;
            fseek(file, ptr, SEEK_SET);

            maxl *= 2;
            fgets(line, maxl, file);
        }

        // write the text into memory
        text = realloc(text, ((*counter)+1)*sizeof(char*));
        length = strlen(line);
        text[*counter] = calloc(sizeof(char), length+1);
        strcpy(text[*counter], line);

        ptr = ftell(file);
        (*counter) ++;
        maxl = 256;
    }
    free(l);
    fclose(file);
	
    return text;
}

void createIndexes(char** text, Trie* trie, int text_num, char* filename, int file_index){

    char* token;
    for (int i = 0; i < text_num; i++) {

        char* tempstr = calloc(strlen(text[i])+1, sizeof(char));
        strcpy(tempstr, text[i]);
		// printf("Worker : %s\n", tempstr);
        token = strtok(tempstr, " \t");
		// printf("Worker token : %s\n", token);

		if(token[strlen(token)-1] == '\n') token[strlen(token)-1] = '\0';

		if(token[0] == '\n' || token[0] == '<' || token == NULL || token[0] == '\0'){
			free(tempstr);
			continue;
		} 

		// printf("Inserting %s\n", token);
        Insert(trie, token, filename, i, file_index);
        // words[i]++;

        while(token = strtok(NULL, " \t")){
			if(token[strlen(token)-1] == '\n') token[strlen(token)-1] = '\0';
			if(token[0] == '\n' || token[0] == '<' || token == NULL || token[0] == '\0'){
				free(tempstr);
				continue;
			}
            Insert(trie, token, filename, i, file_index);
            // words[i]++;
        }
		free(tempstr);
    }
}

void search(char** qs, Trie* root, char*** text, int fd_pipe, int log_fd, char* t){
	Plist* lroot = NULL;
	pid_t pid = getpid();
	int found = 0;
    // for every word on query take the posting lists
    int i=0;
    while((i < 10) && strcmp(qs[i], "\0")){
		// printf("%d searching query %d\n", pid, i);

		// write to logs
		char logs[1024];
		sprintf(logs, "%s : search : %s :", t, qs[i]);
		write(log_fd, logs, strlen(logs)+1);


		// if word found
		// printf("about to search %s\n", qs[i]);
        if(Search(root, qs[i], &lroot)){
			// printf("search found something\n");

			found ++;
			// for all the files in the posting list
			plistNode* temp = lroot->head;
			while(temp != NULL){

				char logs2[1024];
				sprintf(logs2, " %s :", temp->path);
				write(log_fd, logs2, strlen(logs2));

				int line = temp->line[0];
				// send all the occurensies in each file
				for (size_t k = 0; k < temp->df; k++) {

					// don't write the same lines
					if((k>0) && (temp->line[k] == line)) continue;
					line = temp->line[k];
					send_searchAnswer(temp->path, temp->line[k], text[temp->file_index][temp->line[k]], fd_pipe);
				}
				temp = temp->next;
			}
        }
        i++;
		write(log_fd, "\n", 2);
    }

	// if (!found){
	// 	send_searchAnswer(" ", -3, " ", fd_pipe);
	// }
	// send this to signal the end of answering
	send_searchAnswer(" ", -1, " ", fd_pipe);
}

void send_searchAnswer(char* path, int line_num, char* line, int fd_pipe){
	pid_t pid = getpid();
	answer a;
	strcpy(a.path, path);
	a.line_num = line_num;
	strcpy(a.line, line);
	// printf("Answer send from %d : file -> %s line# -> %d, line : %s\n", pid, a.path, a.line_num, a.line);
	mywrite(fd_pipe, (char*)&a, sizeof(answer));
}

int numbers_only(const char* s){
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }
    return 1;
}

int make_fifos(char** name_r, char** name_w, int* fd_r, int* fd_w){
	pid_t pid = getpid();
	make_fifo_name(pid, *name_w, 0);
	while((*fd_w = open(*name_w, O_WRONLY)) < 0){
		// perror("Worker : open write pipe");
		// printf("%s\n", pipe_name_w);
		// exit(-1);
	}

	make_fifo_name(pid, *name_r, 1);
	if(mkfifo(*name_r, 0666) == -1){
		if(errno!=EEXIST){
			perror("jex : mkfifo");
			exit(-1);
		}
	}
	if((*fd_r = open(*name_r, O_RDONLY)) < 0){
		perror("Worker : open read pipe");
		// printf("%s\n", pipe_name_w);
		// exit(-1);
	}
	return 1;
}

int read_paths(int fd, char** paths, int n){
	
	for (size_t i = 0; i < n; i++) {
		int s=0, len=0;
		char temp[10];
		s = read(fd, temp, 2+1);
		// printf("Worker : message %s of size %ld\n", temp, s);
		s = atoi(temp);
		my_read(fd, &paths[i], s+1);
		// printf("Worker : message %s of size %d\n", paths[i], s);
	}
}

int read_files(int n, char** paths, char**** text, int** txt_num, Trie* trie, int* lines, int* file_cnt){

	int max_files = n;
	int index = 0;
	int s = 1;
	*txt_num = malloc(1*sizeof(int));
	(*txt_num)[index] = 0;

	// for all the paths
	for (size_t i = 0; i < n; i++) {
		DIR* dp;
		struct dirent* dir;
		// paths[i][strlen(paths[i])-1] = '\0';

		if((dp = opendir(paths[i])) == NULL){
			perror("opendir");
			printf("%s\n", paths[i]);
			return -1;
		}
		// printf("Worker : In path %s\n", paths[i]);
		// for all the files in each path
		while((dir = readdir(dp)) != NULL){
			if(dir->d_ino == 0) continue;

			char addr[strlen(paths[i])+1];
			strcpy(addr, paths[i]);
			strcat(addr, "/");
			strcat(addr, dir->d_name);

			// if we are checking a directory then continue to the next
			struct stat mybuf;
			if(stat(addr, &mybuf) < 0){
				perror(addr);
				continue;
			}
			// if((mybuf.st_mode & S_IFMT) == S_IFDIR) continue;
			if(S_ISDIR(mybuf.st_mode)) continue;

			(*file_cnt)++;
			if((*file_cnt) > max_files){
				*text = realloc(*text, 2*max_files*sizeof(char**));
				max_files *= 2;
			}

			int txt_num_tmp = 0;
			// printf("Worker : Going to parse %s\n", addr);
			(*text)[(*file_cnt)-1] = parseFile(addr, &txt_num_tmp);
			// printf("Worker : file parsed %s\n", (*text)[(*file_cnt)-1][0]);
			// printf("index : %d num : %d before %d\n", index, txt_num_tmp, (*txt_num)[index]);
			
			(*txt_num)[index] = txt_num_tmp;
			// printf("%d\n", (*txt_num)[index]);
			index++;
			s++;
			
			*txt_num = realloc(*txt_num, s*sizeof(int));
			(*txt_num)[index] = 0;
			// printf("going to create indexes\n");
			createIndexes((*text)[(*file_cnt)-1], trie, txt_num_tmp, addr, (*file_cnt)-1);
			// printf("indexes created\n");
			(*lines) += txt_num_tmp;
		}
	}
}

int read_command(char** buffer, int* command, int fd, char* name){
	char tempc[20] ;
	pid_t pid = getpid();

	if(read(fd, tempc, (20 * sizeof(char))) == -1){
		perror("error Worker : read");
		printf("%d->%s\n", pid, name);
	}

	(*command) = tempc[0] - 48;
	tempc[0] = '0';
	int size = atoi(tempc);

	if((*command) <= 0){
		return -1;
	}
	else if((*command) <= 3){
		*buffer = malloc(size * sizeof(char));
		int s = (size)*sizeof(char);
		int ret;

		// printf("Worker %d : about to read size %d from %s-%d\n", pid, size, name, fd);
		my_read(fd, buffer, s+1);
		// printf("Worker %d : read %s size: %d\n", pid, *buffer, size);

	}
	else if((*command) <= 5){
		*buffer = NULL;
	}

	return size;
}

void seperate_words(char*** qs, char* buffer){
	int i = 0;
	*qs = malloc(MAX_Q_NUM * sizeof(char*));

	char* token = strtok(buffer, " ");
	(*qs)[i] = malloc((strlen(token)+1) * sizeof(char));
	strcpy((*qs)[i], token);

	i++;
	while((token = strtok(NULL, " "))){

		(*qs)[i] = malloc((strlen(token)+1) * sizeof(char));
		strcpy((*qs)[i], token);
		i++;
	}

	// if(!((i < 3) || strcmp((*qs)[i-2], "-d"))){
	// 	i-=2;
	// }

	while(i<10){
		(*qs)[i] = malloc(1);
		(*qs)[i][0] = '\0';
		i++;
	}
}

void send_count_answer(int found, Plist* lroot, int fd_w, int fd_l, char* name, time_t rawtime, char* token, int flag){
	pid_t pid = getpid();
	int min_max = 0;

	if(found){
		printf("Worker %d found the word\n", pid);
		char tmp_path[SIZE_OF_ADDRESS];
		plistNode* templ = lroot->head;
		strcpy(tmp_path, templ->path);
		min_max = templ->df;
		templ = templ->next;
		while(templ != NULL){
			if(flag){
				if(templ->df > min_max){
					min_max = templ->df;
					strcpy(tmp_path, templ->path);
				}
			}
			else{
				if(templ->df < min_max){
					min_max = templ->df;
					strcpy(tmp_path, templ->path);
				}
			}
			templ = templ->next;
		}

		printf("Worker %d : sending %d to %s-%d, %s\n", pid, min_max, name, fd_w, tmp_path);
		answer a;
		strcpy(a.path, tmp_path);
		a.line_num = min_max;
		strcpy(a.line, "f");
		// write(fd_pipe_write, &a, sizeof(answer));
		mywrite(fd_w, (char*)&a, sizeof(answer)+1);

		// write to logs
		char logs[1024];
		char t[20];
		// strcpy(t, asctime (timeinfo));
		sprintf(t, "%ld", rawtime);
		t[strlen(t)-1] = '\0';
		if(flag)
			sprintf(logs, "%s : maxcount : %s : winner file %s (%d)\n", t, token, a.path, min_max);
		else
			sprintf(logs, "%s : mincount : %s : winner file %s (%d)\n", t, token, a.path, min_max);

		write(fd_l, logs, strlen(logs)+1);
		printf("Worker %d finished\n", pid);
	}
	else{
		printf("Worker %d did not found the word\n", pid);
		answer a;
		a.line_num = -1;
		strcpy(a.line, "n");

		// write(fd_pipe_write, &a, sizeof(answer));
		mywrite(fd_w, (char*)&a, sizeof(answer)+1);

		// write to logs
		char logs[1024];
		char t[20];
		// strcpy(t, asctime (timeinfo));
		sprintf(t, "%ld", rawtime);
		t[strlen(t)-1] = '\0';
		if(flag)
			sprintf(logs, "%s : maxcount : %s : \n", t, token);
		else
			sprintf(logs, "%s : mincount : %s : \n", t, token);

		write(fd_l, logs, strlen(logs)+1);
		printf("Worker %d finished\n", pid);
	}
}

int my_read(int fd, char** buffer, int size){
	// *buffer = malloc(size * sizeof(char));
	int len=0, ret=0;

    while(len == 0) ioctl(fd, FIONREAD, &len);
    char* temp = malloc(len * sizeof(char)+1);

    if((ret = read(fd, temp, len)) < 0) perror_exit("read");
    (temp)[ret] = '\0';

	strcpy(*buffer, temp);
	free(temp);
	size -= len;
	while(size>0){
		ioctl(fd, FIONREAD, &len);
        char* temp3 = malloc(len*sizeof(char)+1);
        if((ret = read(fd, temp3, len)) < 0) perror_exit("read");
        (temp3)[ret] = '\0';
        size -= len;
        
        strcat(*buffer, temp3);
		free(temp3);
		// printf("read : %d remaining : %d\n", ret, size);
	}
}

void perror_exit(char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}
