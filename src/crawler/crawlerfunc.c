#include "crawler.h"

char* parseArgs(int argc, char* argv[], int* s_port, int* c_port, int* tr_num, char** ip, char** start_url){

    argv[argc] = NULL;

    // check if we have provided at least all the arguments
    if (argc < 11) perror_exit("Error : Not enough arguments provided, please provide :\n-p <serving port> -c <command port> -t <number of threads> -d <root directory>\n");

    // check for serving port
    int i = 0;
    while(strcmp(argv[i], "-p") != 0){
        i++;
        if(argv[i] == NULL) perror_exit("Error : Serving Port not provided\n");
    }

    if(argv[i+1] == NULL) perror_exit("Error : Serving Port not provided\n");

    *s_port = atoi(argv[i+1]);
    if(*s_port <= 0) perror_exit("Error : Serving Port does not exist\n");

    // check for command port
    i = 0;
    while(strcmp(argv[i], "-c") != 0){
        i++;
        if(argv[i] == NULL) perror_exit("Error : Command Port not provided\n");
    }

    if(argv[i+1] == NULL) perror_exit("Error : Command Port not provided\n");

    *c_port = atoi(argv[i+1]);
    if(*s_port <= 0){
        perror("Error : Command Port does not exist\n");
        exit(-1);
    }

    // check thread number
    i = 0;
    while(strcmp(argv[i], "-t") != 0){
        i++;
        if(argv[i] == NULL) perror("Error : Number of threads not provided\n");
    }

    if(argv[i+1] == NULL) perror("Error : Number of threads not provided\n");

    *tr_num = atoi(argv[i+1]);
    if(*s_port <= 0) perror_exit("Error : Number of threads must be positive\n");

    // check for server ip or name
    i = 0;
    while(strcmp(argv[i], "-h") != 0){
        i++;
        if(argv[i] == NULL) perror_exit("Error : Server ip not provided\n");
    }

    if(argv[i+1] == NULL) perror_exit("Error : Server ip not provided\n");
    *ip = argv[i+1];

    // check for Starting URL
    // DIR* di;
    // if((di = opendir(argv[argc-1])) == NULL) perror_exit("Given starting url");
    // closedir(di);
    *start_url = argv[argc-1];

    // check for save directory
    i = 0;
    while(strcmp(argv[i], "-d") != 0){
        i++;
        if(argv[i] == NULL) perror_exit("Error : Save directory not provided\n");
    }

    if(argv[i+1] == NULL) perror_exit("Error : Save directory not provided\n");

    DIR* d;
    if((d = opendir(argv[i+1])) == NULL) perror_exit("Given Save Directory");
    /* delete all previus files inside */
    struct dirent* dir;
    while((dir = readdir(d)) != NULL){
        if(dir->d_ino == 0) continue;
        if(strcmp(dir->d_name, "..") && strcmp(dir->d_name, ".")){
            char temp[263];
            sprintf(temp, "rm -r %s/%s", argv[i+1], dir->d_name);
            system(temp);
        }
    }
    closedir(d);

    return argv[i+1];
}

void perror_exit(char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int sock_Init(int port, struct sockaddr_in* server, char* str_ip, const char* type){
    int sock;
    memset(server, '0', sizeof(server));
    if(( sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) perror_exit("socket");  
    server->sin_family = AF_INET; /* Internet domain */
    server->sin_port = htons(port); /* The given port */

    if(strcmp(type, "c") == 0)
        server->sin_addr.s_addr = htonl(INADDR_ANY);
    else
        to_ip(server, str_ip, &server->sin_addr.s_addr);
    
    return sock;
}

int isIp_v4( char* ip){
    int num;
    int tflag = 1;
    int counter=0;
    char temp[100];
    temp[0] = '\0';
    strcpy(temp, ip);
    char* p = strtok(temp,".");

    while (p && tflag ){
        num = atoi(p);

        if (num>=0 && num<=255 && (counter++<4)){
            tflag=1;
            p=strtok(NULL,".");
        }
        else{
            tflag=0;
            break;
        }
    }

    if(tflag && (counter == 4)) return 1;
    return 0;

}

int hostname_to_ip(char* hostname, in_addr_t* addr){
    struct hostent* he;
    struct in_addr** addr_list;
    int i;
    char ip[30];

    if((he = gethostbyname(hostname)) == NULL){
        herror("gethostbyname");
        exit(1);
    }

    addr_list = (struct in_addr**) he->h_addr_list;
    strcpy(ip, inet_ntoa(*addr_list[0]));
    *addr = inet_addr(ip);
    // free(hostname);
}

int ip_to_ip(char* str_ip, in_addr_t* addr){
    *addr = inet_addr(str_ip);
}

void to_ip(struct sockaddr_in* server, char* str, in_addr_t* addr){
    /* Find server address */
      
    if(isIp_v4(str)){
        ip_to_ip(str, &server->sin_addr.s_addr);
        // printf("string ip %s translated to %s\n", str, inet_ntoa(server->sin_addr));
    }
    else{
        hostname_to_ip(str, &server->sin_addr.s_addr);
        // printf("hostname %s translated to %s\n", str, inet_ntoa(server->sin_addr));
    }
}

void command_handler(int sock_c, struct timeval* start_time, Queue* queue, char* new_dir){
    int newsock, err;
    struct sockaddr_in client_c;
    socklen_t client_clen ;
    struct sockaddr * client_cptr =( struct sockaddr *) & client_c ;

    /* accept new connection */
    if((newsock = accept(sock_c, NULL, NULL)) < 0) perror_exit("accept");
    // printf("Accepted Connection\n");

    /* read the Command */
    char* buf; 
    my_read2(&buf, newsock);
    buf[strlen(buf)-2] = '\0';

    /* Parse Command */
    if(strcmp(buf, "shutdown") == 0){
        flag = 0;
    }
    else if(strcmp(buf, "stats") == 0){
        struct timeval c_time, n_time;
        gettimeofday(&c_time, NULL);
        timersub(&c_time, start_time, &n_time);
        char str[100];
        sprintf(str, "Server up for %ld:%ld.%.2ld, served %d pages, %lld bytes\n", n_time.tv_sec/60, n_time.tv_sec%60, n_time.tv_usec/10000, pages, bytes);
        if((err = write(newsock, str, strlen(str)*sizeof(char))) < 0) perror_exit("write");
    }
    else{
        char* token = strtok(buf, " ");
        if(strcmp(token, "search") == 0){

            if(qEmpty(queue)){
                // printf("searching\n");
                token = strtok(NULL, "\n");
                jobExecutor(new_dir, newsock, token);
            }
            else{
                char str[100];
                sprintf(str, "Crawling still in progress...\n");
                if((err = write(newsock, str, strlen(str)*sizeof(char))) < 0) perror_exit("write");
            }
        }
        else{
            char str[100];
            sprintf(str, "Invalid Command\n");
            if((err = write(newsock, str, strlen(str)*sizeof(char))) < 0) perror_exit("write");
        }
    }

    /* close socket */
    // printf("Closing Connection\n");
    free(buf);
    close(newsock);
}

void my_read2(char** buf, int fd){

    int len=0;
    while(len == 0) ioctl(fd, FIONREAD, &len);
    *buf = malloc((len+1) * sizeof(char));

    int r=0;
    if((r = read(fd, *buf, len+1)) < 0) perror_exit("read");
    (*buf)[r] = '\0';
}

char** extract_paths(char* new_dir){
    DIR* dir;
    struct dirent* ent;

    char** paths = malloc(sizeof(char*));
    int i=0;
    

    if((dir = opendir("./new_websites/")) == NULL) perror_exit("dir open");

    while((ent = readdir(dir)) != NULL){
        struct stat mybuf;
        char* temp = malloc(150);
        strcpy(temp, "./new_websites/");
        strcat(temp, ent->d_name);
        
        if(stat(temp, &mybuf) < 0){
            perror(ent->d_name);
            continue;
        }
        if((strcmp(temp, "./new_websites/..") == 0) || (strcmp(temp, "./new_websites/.") == 0)) {
            free(temp);
            continue;
        }
        
        if(S_ISDIR(mybuf.st_mode)){
            paths[i] = malloc(150*sizeof(char));
            strcpy(paths[i], temp);
            paths = realloc(paths, (i+1)*sizeof(char*));
        }
        
        free(temp);
        i++;
    }
    path_num = i;
    
    return paths;
}

int make_worker(int n){
	pid_t pid;
	if((pid = fork()) < 0){
		perror("Could not fork");
		exit(-1);
	}

	if(pid == 0){
		char* m = malloc(4);
		sprintf(m, "%d", n);
        // printf("about to make worker\n");
		if(execl("worker", "worker", m, (char*)0) == -1){
			perror("Cannot execute Worker");
			return 0;
		}
        // printf("here\n");
		_exit(127);
	}

	return pid;
}

int make_fifos(int pid, char** name_r, char** name_w, int* fd_r, int* fd_w){
	// make the reading (for jexec) fifo
	make_fifo_name(pid, *name_r, 0);
	if(mkfifo(*name_r, 0666) == -1){
		if(errno!=EEXIST){
			perror("jex : mkfifo");
			exit(-1);
		}
	}

	if((*fd_r = open(*name_r, O_RDONLY)) == -1){
		perror("jex : open read pipe");
		// exit(-1);
	}

	make_fifo_name(pid, *name_w, 1);

	while((*fd_w = open(*name_w, O_WRONLY)) == -1){
		// perror("jex : open write pipe");
		// printf("%s\n", pipe_w[i]);
		// exit(-1);
	}
	return 1;
}

int share_paths(int fd, char** paths, char* name, int* k, int n){
	int c, s;
	for(int j=0; j<n; j++){
		s = strlen(paths[*k]);
        char temp[10];
        sprintf(temp, "%d", s);
		c = write(fd, temp, 2+1);
        // printf("jex : send %s to %s-%d. written : %d\n", temp, name, fd, c);
		c = mywrite(fd, paths[*k], s+1);
		// printf("jex : writing %s to %s-%d. written : %d\n", paths[*k], name, fd, s);
		(*k)++;
        sleep(1);
	}
}

int send_command(int command, int len, int fd, char* buffer, char* name){
	int c, s;
	char temp[20];
	sprintf(temp, "%d%d", command, len);

	// send the command along with the size of keyword(s)
	s = (strlen(temp))*sizeof(char);
	c = mywrite(fd, temp, 20*sizeof(char));
	// printf("jex : has written %s to %s-%d. size : %d\n", temp, name, fd, c);

	if(buffer != NULL){
		// send the keyword(s)
		s = (len) * sizeof(char);
		c = mywrite(fd, buffer, s+1);
		// printf("jex : has written %s to %s-%d. size : %d\n", buffer, name, fd, c);
	}
}

void poll_Init(struct pollfd* fdr, int* fd){
	for (size_t i = 0; i < W; i++) {
		fdr[i].fd = fd[i];
		fdr[i].events = POLLIN;
		fdr[i].revents = 0;
	}
}

int check_time(int pid, int start_time, int deadline){
	time_t rawtime;
	time(&rawtime);
	if(rawtime-start_time > deadline){
		printf("Worker %d timed out\n", pid);
		return 1;
	}
	return 0;
}

void jobExecutor(char* new_dir, int sock_c, char* buffer){
    char** pipe_r;
    char** pipe_w;
    int* fd_pipe_read;
    int* fd_pipe_write;
    int* pids;
    int ret;
    time_t start_time;
    struct pollfd* fdr = malloc(W*sizeof(fdr));

    //store the paths into memory and find the total path_num
    char** paths = extract_paths(new_dir);

    int w = W;
    if(w>path_num) w = path_num;

    // the share of each worker
	int n = path_num/w;

    fd_pipe_read = malloc(w * sizeof(int));
	fd_pipe_write = malloc(w * sizeof(int));
	pipe_r = malloc(w * sizeof(char*));
	pipe_w = malloc(w * sizeof(char*));
	pids = malloc(w * sizeof(int));

    for (size_t i = 0; i < w; i++) {
		pipe_r[i] = calloc(20, sizeof(char));
		pipe_w[i] = calloc(20, sizeof(char));
	}

    // printf("making workers\n");
    // make workers, fifos and share the paths
	int k = 0;
	for (size_t i = 0; i < w; i++) {
		pids[i] = make_worker(n);
        // printf("1\n");
		make_fifos(pids[i], &pipe_r[i], &pipe_w[i], &fd_pipe_read[i], &fd_pipe_write[i]);
        // printf("2\n");
		share_paths(fd_pipe_write[i], paths, pipe_w[i], &k, n);
        // printf("3\n");
	}
    
    // process the command given
    size_t len = strlen(buffer);
    poll_Init(fdr, fd_pipe_read);
    int timeout = 0;
    // printf("sending command\n");
    // send the command along with the word to workers
    for (size_t i = 0; i < w; i++) {
        send_command(1, len, fd_pipe_write[i], buffer, pipe_w[i]);
    }

    time(&start_time);
    int deadline = DEADLINE;
    // printf("reading results\n");
    // read the answers
    for (size_t i = 0; i < w; i++) {

        int j = 0;
        answer a;

        int ret = poll(fdr, w, -1);
        if(ret>0){

            int index = -1;
            for (size_t y = 0; y < w; y++) {
                if(fdr[y].revents & POLLIN){
                    index = y;
                    fdr[y].fd = -1;
                    break;
                }
            }

            do{
                // check the time
                timeout = check_time(pids[index], start_time, deadline);

                if(!timeout){
                    a.line_num = -2;
                    // printf("jex : about to read from %s-%d\n", pipe_r[index], fd_pipe_read[index]);
                    if((ret = read(fd_pipe_read[index], &a, sizeof(answer))) == -1){
                        perror("jex : read");
                        printf("%s\n", pipe_w[index]);
                    }
                    // printf("jex : has read from %s-%d. size : %d\n", pipe_r[i], fd_pipe_read[i], ret);

                    if((a.line_num == -1) && j == 0){
                        char str[50];
                        int err;
                        sprintf(str, "keywords not found \n");
                        if((err = write(sock_c, str, strlen(str)*sizeof(char))) < 0) perror_exit("write");
                        // printf("Answer received from %d : keywords not found \n", pids[index]);
                        break;
                    }
                    j++;

                    if(a.line_num != -1){
                        char str[1500];
                        int err;
                        sprintf(str, "file -> %s line# -> %d, line : %s\n", a.path, a.line_num, a.line);
                        if((err = write(sock_c, str, strlen(str)*sizeof(char))) < 0) perror_exit("write");
                        // printf("Answer received from %d : file -> %s line# -> %d, line : %s\n", pids[index], a.path, a.line_num, a.line);
                    }
                }
                else{
                    if(read(fd_pipe_read[index], &a, sizeof(answer)) == -1){
                        perror("jex : read");
                        printf("%s\n", pipe_w[index]);
                    }
                }

            }while(a.line_num != -1);
        }
        else{
            perror("poll failed");
        }
    }

    // free all structures
	for (size_t i = 0; i < w; i++) {
		unlink(pipe_r[i]);
		unlink(pipe_w[i]);
		free(pipe_r[i]);
		free(pipe_w[i]);
		close(fd_pipe_read[i]);
		close(fd_pipe_write[i]);
	}

    for (size_t i = 0; i < path_num; i++) {
		free(paths[i]);
	}
	free(paths);
	free(pipe_r);
	free(pipe_w);
	free(fd_pipe_read);
	free(fd_pipe_write);
	free(pids);
	free(fdr);

}