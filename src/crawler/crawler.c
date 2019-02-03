#include "crawler.h"

sem_t sem;
pthread_mutex_t ac = PTHREAD_MUTEX_INITIALIZER;
char** visited;
int count=0, pages;
long long int bytes;
int path_num = 0;
int flag = 1;
 
int main(int argc, char *argv[]){
    int s_port, c_port, tr_num, sock_c, ret;
    char* ip, *start_url;
    struct sockaddr_in server_c;
    struct sockaddr * server_cptr =( struct sockaddr *) & server_c ;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    pages = 0;
    bytes = 0;

    /* parse the arguments given */
    char* new_dir = parseArgs(argc, argv, &s_port, &c_port, &tr_num, &ip, &start_url);

    /* Create a Queue for the GET Requests */
    Queue* queue = createQueue();

    /* Initialize counting semaphore */
    ret = sem_init(&sem, 0, count);

    /* Create sockets and Initialize structs */
    sock_c = sock_Init(c_port, &server_c, ip, "c");

    /* make sure the socket will be reusable */
    if (setsockopt(sock_c, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) perror_exit("setsockopt(SO_REUSEADDR)");

    /* Initialize the Thread Pool */
    thr_pool* pool = thr_pool_Init(tr_num, queue, new_dir, s_port, ip);

    /* Bind socket to address */
    if( bind(sock_c ,server_cptr ,sizeof(server_c)) < 0) perror_exit("bind");

    /* Listen for connections */ 
    if( listen(sock_c ,125) < 0) perror_exit("listen");
    printf("Listening for commands to port %d\n", c_port);

    /* Insert the first request and raise the semaphore */
    enQueue(queue, start_url, 200);
    char temp[strlen(start_url)+1];
    strcpy(temp, start_url);
    char* token = strtok(temp, "/");
    token = strtok(NULL, "/");
    token = strtok(NULL, "\n");
    char temp2[strlen(token)+2];
    sprintf(temp2, "/%s", token);
    link_nvisited(temp2);
    
    ret = sem_post(&sem);

    
    while(flag){
        command_handler(sock_c, &start_time, queue, new_dir);

        /* check if thread terminated */    
        for(size_t i = 0; i < tr_num; i++){
            if(pthread_tryjoin_np(pool->thr[i], NULL) == 0)
                thr_restart(queue, pool, i, new_dir, s_port, ip);
        }
        
    }
    
    for(size_t i = 0; i <= tr_num; i++){
        sem_post(&sem);
    }

    
    for(size_t i = 0; i < tr_num; i++){
        pthread_join(pool->thr[i], NULL);
        
    }
    
    free(pool->thr);
    free(pool);
    for(int i=0; i<count; i++){
        free(visited[i]);
    }
    free(visited);
    destroyqueue(queue);
    free(queue);
    return 0;
}
