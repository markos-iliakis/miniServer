#include "threadpool.h"

thr_pool* thr_pool_Init(int num, Queue* queue, char* new_dir, int s_port, char* ip){
    int err;
    flag2 = 0;
    thr_pool* pool = malloc(sizeof(thr_pool));
    pool->thr = malloc(num * sizeof(pthread_t*));
    args* ar = args_Init(queue, new_dir, s_port, ip);
    count = 0;

    for(int i=0; i < num; i++){
        if((err = pthread_create(&(pool->thr[i]), NULL, (void*)request, (void*)ar)) != 0) perror2("pthread_create", err);
        // printf("thread %d created\n", i);
    }

    pool->thr_num = num;
    return pool;
}

void* request(void* ar){
    args* arg = ar;
    int err, ret, link_num=0, sock;
    
    QNode* qn;

    while(flag){
        char* buf;
        // printf("waiting c_sem\n");
        /* counting semaphore down */
        ret = sem_wait(&sem);

        if(!flag) break;
        // printf("waiting sem\n");
        /* mutex lock */
        if((err = pthread_mutex_lock(&ac)) != 0) perror2("mutex lock", err);

        /* take element from queue */
        if(qEmpty(arg->queue)) printf("Problemmm\n");
        qn = deQueue(arg->queue);
        pages++;

        /* mutex unlock */
        if((err = pthread_mutex_unlock(&ac)) != 0) perror2("mutex unlock", err);

        /* make the socket and connect */
        sock = my_connect(arg->port, arg->ip);

        /* forge and send the request */
        char* req = forge_request(qn->key);
        int len = strlen(req);
// printf("1\n");
        if((err=write(sock, req, len)) < 0) perror2("write", err);
// printf("2\n");
        /* receive and analyze page */
        // printf("analyzing\n");
        my_read(&buf, sock);
        char** links = extract_links(buf, &link_num, arg->ip, arg->port);
// printf("3\n");
        /* save the page to save_dir */
        create_file(arg->dir, qn->key, buf);
// printf("4\n");
        // printf("waiting sem down\n");
        /* mutex lock */
        if((err = pthread_mutex_lock(&ac)) != 0) perror2("mutex lock", err);

        /* for all the links in the site */
        for(int i=0; i < link_num; i++){
            
            /* add to queue */
            enQueue(arg->queue, links[i], 200);

            /* counting semaphore up */
            ret = sem_post(&sem);
        }

        /* mutex unlock */
        if((err = pthread_mutex_unlock(&ac)) != 0) perror2("mutex unlock", err);

        close(sock);
        free(req);
        req = NULL;      
        for(size_t i = 0; i < link_num; i++){
            free(links[i]);
        }   
        free(links);
        links = NULL;
        free(qn->key);
        free(qn);
        qn = NULL;
        free(buf);
        buf = NULL;
        // sleep(1);
    }
    /* mutex lock */
    if((err = pthread_mutex_lock(&ac)) != 0) perror2("mutex lock", err);
    if(!flag2){
        free(arg);
        flag2 = 1;
    }
    /* mutex unlock */
    if((err = pthread_mutex_unlock(&ac)) != 0) perror2("mutex unlock", err);
    printf("thread exiting\n");
}

void perror2(const char* s, int err){
    fprintf(stderr, "%s: %s\n", s, strerror(err));
    exit(1);
}

char* forge_request(char* url){
    char* req = malloc(500*sizeof(char));
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    // printf("Hostname: %s\n", hostname);
    struct hostent* h;
    h = gethostbyname(hostname);
    // printf("h_name: %s\n", h->h_name);

    // printf("url : %s\n", url);

    char temp[strlen(url)+1];
    strcpy(temp, url);
    char* token = strtok(temp, "/");
    token = strtok(NULL, "/");
    // token = strtok(NULL, "/");
    token = strtok(NULL, "\n");
        
    sprintf(req, "GET /%s HTTP/1.1\nHost: %s\n\n", token, h->h_name);
    // printf("request for %s\n", req);

    return req;
}

void my_read(char** buf, int fd){
    int len=0, tot_len=0, r;

    while(len == 0) ioctl(fd, FIONREAD, &len);
    // char* temp = malloc(len * sizeof(char)+1);
    // char* temp2 = malloc(len * sizeof(char)+1);
    char temp[len+1];
    char temp2[len+1];

    /* Get the first n bytes transfered */
    // printf("about to read\n");
    if((r = read(fd, temp, len+1)) < 0) perror2("read", r);
    // printf("read\n");
    (temp)[r] = '\0';
    strcpy(temp2, temp);

    /* Check the Header for 200 OK and total len */
    // char* header = malloc(100*sizeof(char));
    char* token = strtok(temp, "\n");
    if(strcmp(token, "HTTP/1.1 200 OK") != 0){
        if(strcmp(token, "HTTP/1.1 404 Not Found") == 0)
            perror2("page not found\n", -1);
        else
            perror2("forbidden page\n", -1);
    }

    
    for(int i = 0; i < 3; i++){
        token = strtok(NULL, "\n");
    }
    
    char* tok2 = strtok(token, " ");
    tok2 = strtok(NULL, " ");
    tot_len = atoi(tok2);
    bytes += tot_len;
    // printf("total len : %d\n", tot_len);
    
    /* dont copy the header */
    int j=0;
    while(temp2[j] != '<'){
        j++;
    }
    len -= j;
    

    // printf("read : %d remaining : %d\n", len, tot_len);
    *buf = malloc(tot_len*sizeof(char)+1);
    tot_len -= len;
    strcpy(*buf, temp2+j);
    // free(temp);
    // free(temp2);
    
    /* if total length greater than what we have read, keep reading */
    while(tot_len>0){

        ioctl(fd, FIONREAD, &len);
        char* temp3 = malloc(len*sizeof(char)+1);
        if((r = read(fd, temp3, len+1)) < 0) perror2("read", r);
        (temp3)[r] = '\0';
        tot_len -= len;
        
        strcat(*buf, temp3);
        // printf("read : %d remaining : %d\n", r, tot_len);
    }
    
}

char** extract_links(char* buf, int* link_num, char* ip, int port){

    char** links = malloc(5*sizeof(char*));
    char temp[100];
    int flag=0, j=0, lnk_index=0, max_lnk=4;

    for(int i=0; i < strlen(buf); i++){
        
        if(buf[i] == '<'){
            flag = 1;
            temp[j] = buf[i];
            j++;
        }
        else if(buf[i] == '>'){
            flag = 0;
            temp[j] = buf[i];
            temp[j+1] = '\0';

            if(strncmp(temp, "<a href=", 8) == 0){
                temp[strlen(temp)-1] = '\0';

                if(link_nvisited(temp+8)){

                    if(lnk_index > max_lnk){
                        links = realloc(links, ++max_lnk * sizeof(char*));
                    }
                    char prt[6];
                    sprintf(prt, "%d", port);
                    links[lnk_index] = calloc(strlen(temp)+strlen(ip)+strlen(prt)+10, sizeof(char));
                    sprintf(links[lnk_index++], "http://%s:%s%s", ip, prt, temp+8);
                    // strcpy(links[lnk_index++], temp+8);
                    // printf("link found : %s\n", links[lnk_index-1]);
                }
            }
            j=0;
        }
        else if(flag){
            temp[j] = buf[i];
            j++;
        } 
    }
    *link_num = lnk_index;
    return links;
}

void create_file(char* str1, char* str2, char* buf){
    char pathFile[200];
    int ret, fd;
    char temp1[strlen(str2)+1];
    strcpy(temp1, str2);
    char* token = strtok(temp1, "/");
    token = strtok(NULL, "/");
    // token = strtok(NULL, "/");
    token = strtok(NULL, "\n");
    // printf(" token : %s\n", str2);

    sprintf(pathFile, "%s/%s", str1, token);
    // printf(" path %s\n", pathFile);
    pathFile[strlen(pathFile)] = '\0';
    char temp[strlen(pathFile)+1];
    strcpy(temp, pathFile);

    /* create folder if doesn't exist */
    int i = 0, j=0;
    while(pathFile[i] != '\0'){
        if(pathFile[i] == '/') j = i;
        i++;
    }
    temp[j] = '\0';
    
    ret = mkdir(temp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if((fd = open(pathFile, O_RDWR | O_APPEND | O_CREAT)) == -1) perror2("open", fd);
    chmod(pathFile, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    write(fd, buf, strlen(buf));
    close(fd);
}

args* args_Init(Queue* queue, char* new_dir, int s_port, char* ip){
    args* ar = malloc(sizeof(args));
    ar->queue = queue;
    ar->dir = new_dir;
    ar->port = s_port;
    ar->ip = ip;
    return ar;
}

int my_connect(int s_port, char* ip){
    int sock, err;
    struct sockaddr_in server;
    struct sockaddr * serverptr =( struct sockaddr *) & server ;

    /* Create sockets and Initialize structs */
    sock = sock_Init(s_port, &server, ip, "s");

    /* make sure the socket will be reusable */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) perror2("setsockopt(SO_REUSEADDR)", -1);

    /* Initialize connection */
    // printf("trying to connect\n");
    if((err = connect(sock, serverptr, sizeof(*serverptr))) < 0) perror2("connect", err);
    // printf("Connecting to server\n");

    return sock;
}

int link_nvisited(char* link){
    int i = 0;
    
    while(i<count){
        // printf("checking: %s - %s\n", visited[i], link);
        if(strcmp(visited[i], link) == 0){
            return 0;
        }
        i++;
    }
    // printf("found link : %s\n", link);
    // for(int j=0; j < count; j++){
    //     printf("visited : %s\n", visited[j]);
    // }
    count++;
    visited = realloc(visited, count*sizeof(char*));
    visited[i] = malloc(strlen(link)*sizeof(char)+1);
    strcpy(visited[i], link);
    return 1;
}

void thr_restart(Queue* queue, thr_pool* pool, int i, char* new_dir, int s_port, char* ip){
    int err;
    args* ar = args_Init(queue, new_dir, s_port, ip);
    if((err = pthread_create(&(pool->thr[i]), NULL, (void*)request, (void*)ar)) != 0) perror2("pthread_create", err);
}