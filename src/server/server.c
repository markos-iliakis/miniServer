#include "server.h"

pthread_mutex_t ac = PTHREAD_MUTEX_INITIALIZER;
sem_t sem;
int count = 0;
int pages;
long long int bytes;
int flag = 1;

int main(int argc, char *argv[]) {

    int s_port, c_port, tr_num, sock, sock_c, ret;
    pages=0;
    bytes=0;
    struct sockaddr_in server , server_c;
    struct sockaddr * serverptr =( struct sockaddr *) & server ;
    struct sockaddr * server_cptr =( struct sockaddr *) & server_c ;
    struct pollfd* fdr = malloc(2*sizeof(fdr));
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    /* parse the arguments given */
    char* root_dir = parseArgs(argc, argv, &s_port, &c_port, &tr_num);

    /* Initialize counting semaphore */
    ret = sem_init(&sem, 0, count);

    /* Create a Queue for the GET Requests */
    Queue* queue = createQueue();

    /* Initialize the Thread Pool */
    thr_pool* pool = thr_pool_Init(tr_num, queue, root_dir);

    /* Create sockets and Initialize structs */
    sock = sock_Init(s_port, &server);
    sock_c = sock_Init(c_port, &server_c);

    /* make sure the socket will be reusable */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) perror_exit("setsockopt(SO_REUSEADDR)");
    if (setsockopt(sock_c, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) perror_exit("setsockopt(SO_REUSEADDR)");

    /* Bind socket to address */
    if( bind(sock ,serverptr ,sizeof(server)) < 0) perror_exit("bind");
    if( bind(sock_c ,server_cptr ,sizeof(server)) < 0) perror_exit("bind");

    /* Listen for connections */
    if( listen(sock ,125) < 0) perror_exit("listen");  
    if( listen(sock_c ,125) < 0) perror_exit("listen");
    printf("Listening for connections to port %d and commands to port %d Adress: %s\n", s_port, c_port, inet_ntoa(server.sin_addr));
    

    while(flag){

        /* Initialize poll */
        poll_Init(fdr, sock, sock_c);

        /* Poll for GET or command */
        if((ret = poll(fdr, 2, -1)) <= 0) perror_exit("poll");

        if(fdr[0].revents & POLLIN){        /* Handle GET Request */
            GET_handler(sock, root_dir, &pages, &bytes, queue);
        }
        else if(fdr[1].revents & POLLIN){   /* Handle Command */
            flag = command_handler(sock_c, &start_time, pages, bytes);
        }
        else{
            printf("Problem \n");
        }

        /* check if thread terminated */    
        for(size_t i = 0; i < tr_num; i++){
            if(pthread_tryjoin_np(pool->thr[i], NULL) == 0)
                thr_restart(queue, pool, i);
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
    free(queue);
    free(fdr);
    
    return 0;
}