#include "server.h"

char* parseArgs(int argc, char* argv[], int* s_port, int* c_port, int* tr_num){

    argv[argc] = NULL;

    // check if we have provided at least all the arguments
    if (argc < 8){
        perror("Error : Not enough arguments provided, please provide :\n-p <serving port> -c <command port> -t <number of threads> -d <root directory>\n");
        exit(-1);
    }

    // check for serving port
    int i = 0;
    while(strcmp(argv[i], "-p") != 0){
        i++;
        if(argv[i] == NULL){
            perror("Error : Serving Port not provided\n");
            exit(-1);
        }
    }

    if(argv[i+1] == NULL){
        perror("Error : Serving Port not provided\n");
        exit(-1);
    }

    *s_port = atoi(argv[i+1]);
    if(*s_port <= 0){
        perror("Error : Serving Port does not exist\n");
        exit(-1);
    }

    // check for command port
    i = 0;
    while(strcmp(argv[i], "-c") != 0){
        i++;
        if(argv[i] == NULL){
            perror("Error : Command Port not provided\n");
            exit(-1);
        }
    }

    if(argv[i+1] == NULL){
        perror("Error : Command Port not provided\n");
        exit(-1);
    }

    *c_port = atoi(argv[i+1]);
    if(*s_port <= 0){
        perror("Error : Command Port does not exist\n");
        exit(-1);
    }

    // check thread number
    i = 0;
    while(strcmp(argv[i], "-t") != 0){
        i++;
        if(argv[i] == NULL){
            perror("Error : Number of threads not provided\n");
            exit(-1);
        }
    }

    if(argv[i+1] == NULL){
        perror("Error : Number of threads not provided\n");
        exit(-1);
    }

    *tr_num = atoi(argv[i+1]);
    if(*s_port <= 0){
        perror("Error : Number of threads must be positive\n");
        exit(-1);
    }

    // check for root directory
    i = 0;
    while(strcmp(argv[i], "-d") != 0){
        i++;
        if(argv[i] == NULL){
            perror("Error : Root directory not provided\n");
            exit(-1);
        }
    }

    if(argv[i+1] == NULL){
        perror("Error : Root directory not provided\n");
        exit(-1);
    }

    DIR* d;
    if((d = opendir(argv[i+1])) == NULL){
        perror("given directory");
        exit(-1);
    }
    closedir(d);

    return argv[i+1];
}

void perror_exit(char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void my_read(char** buf, int fd){

    int len=0;
    while(len == 0) ioctl(fd, FIONREAD, &len);
    *buf = malloc(len * sizeof(char)+1);

    int r=0;
    if((r = read(fd, *buf, len+1)) < 0) perror_exit("read");
    (*buf)[r] = '\0';
}

off_t fsize(char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}

void poll_Init(struct pollfd* fdr, int fd, int fd2){
	
    fdr[0].fd = fd;
    fdr[0].events = POLLIN;
    fdr[0].revents = 0;
    fdr[1].fd = fd2;
    fdr[1].events = POLLIN;
    fdr[1].revents = 0;	
}

void GET_handler(int sock, char* root_dir, int* pages, long long int* bytes, Queue* queue){
    int newsock, client, err;
    socklen_t clientlen;
    struct sockaddr * clientptr =( struct sockaddr *) & client ;
    
    /* accept new connection */
    if((newsock = accept(sock, NULL, NULL)) < 0) perror_exit("accept");

    /* read the request */
    char* buf; 
    my_read(&buf, newsock);
    printf("Request : \n%s\n", buf);

    if(valid(buf)){
        /* mutex lock */
        if((err = pthread_mutex_lock(&ac)) != 0) perror2("mutex lock", err);

        /* Insert Get Request into linked list queue */
        enQueue(queue, buf, newsock);
        
        /* counting semaphore up */
        err = sem_post(&sem);

        /* mutex unlock */
        if((err = pthread_mutex_unlock(&ac)) != 0) perror2("mutex unlock", err);
    }
    else{
        printf("invalid request\n");
        close(newsock);
    }

    free(buf);
}

int command_handler(int sock_c, struct timeval* start_time, int pages, long long int bytes){
    int newsock, flag=1, ret;
    struct sockaddr_in client_c;
    socklen_t client_clen ;
    struct sockaddr * client_cptr =( struct sockaddr *) & client_c ;

    /* accept new connection */
    if((newsock = accept(sock_c, NULL, NULL)) < 0) perror_exit("accept");
    // printf("Accepted Connection\n");

    /* read the Command */
    char* buf; 
    my_read(&buf, newsock);
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
        if((ret = write(newsock, str, strlen(str)*sizeof(char))) < 0) perror_exit("write");
    }
    else{
        char str[100];
        sprintf(str, "Invalid Command\n");
        if((ret = write(newsock, str, strlen(str)*sizeof(char))) < 0) perror_exit("write");
    }

    /* close socket */
    // printf("Closing Connection\n");
    free(buf);
    close(newsock);
    return flag;
}

int sock_Init(int port, struct sockaddr_in* server){
    int sock;
    if(( sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) perror_exit("socket");  
    server->sin_family = AF_INET; /* Internet domain */
    server->sin_addr.s_addr = htonl(INADDR_ANY);
    server->sin_port = htons(port); /* The given port */
    return sock;
}

int valid(char* req){
    int get=0, host=0;
    char* temp = malloc(strlen(req)+1);
    strcpy(temp, req);

    char* token = strtok(temp, "\n");
    if(strncmp(token, "GET", 3)==0) get=1;
    if(strncmp(token, "Host", 3)==0) host=1;
    
    while(token=strtok(NULL, "\n")){
        if(strncmp(token, "GET", 3)==0) get=1;
        if(strncmp(token, "Host:", 5)==0) host=1;
    }
    free(temp);
    return get&&host;
}