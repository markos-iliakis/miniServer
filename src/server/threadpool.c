#include "threadpool.h"
#include <dirent.h>

thr_pool* thr_pool_Init(int num, Queue* queue, char* root_dir){
    int err;
    flag2 = 1;
    thr_pool* pool = malloc(sizeof(thr_pool));
    pool->thr = malloc(num * sizeof(pthread_t*));
    args* ar = args_Init(queue, root_dir);

    for(int i=0; i < num; i++){
        if((err = pthread_create(&(pool->thr[i]), NULL, (void*)response, (void*)ar)) != 0) perror2("pthread_create", err);
        // printf("thread %d created\n", i);
    }

    pool->thr_num = num;
    return pool;
}

void thr_restart(Queue* queue, thr_pool* pool, int i){
    int err;
    if((err = pthread_create(&(pool->thr[i]), NULL, (void*)response, (void*)queue)) != 0) perror2("pthread_create", err);
}

void my_write(char* page, int fd, char* head){

    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    int page_len = strlen(page);
    char header[150+page_len];
    sprintf(header, "%sDate: %sServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: %d\nContent-Type: text/html\nConnection: Closed\n\n%s", head, asctime(timeinfo), page_len, page);
    int len = strlen(header);
    int w=0;
    
    // printf("Sending Response : %s\n", header);
    if((w=write(fd, header, len+1)) < 0) perror2("write", w);
}

void* response(void* ar){
    int err;
    args* arg = ar;

    QNode* qn;

    while(flag){
        

        /* counting semaphore down */
        err = sem_wait(&sem);

        if(!flag) break;

        /* mutex lock */
        if((err = pthread_mutex_lock(&ac)) != 0) perror2("mutex lock", err);

        qn = deQueue(arg->queue);

        /* mutex unlock */
        if((err = pthread_mutex_unlock(&ac)) != 0) perror2("mutex unlock", err);

        /* Get the page requested */
        char* page = NULL;
        int ret = 0;
        ret = get_page(&page, arg->dir, qn->key);

        char* head = calloc(26,sizeof(char));
        if(ret == 200){    
            strcpy(head, "HTTP/1.1 200 OK\n");
            my_write(page, qn->fd, head);
        }
        else if(ret == 404){
            strcpy(head, "HTTP/1.1 404 Not Found\n");
            my_write(page, qn->fd, head);
        }
        else if(ret == 403){
            strcpy(head, "HTTP/1.1 403 Forbidden\n");
            my_write(page, qn->fd, head);
        }
        else{
            printf("Problem\n");
        }

        /* close socket */
        free(qn->key);
        close(qn->fd);

        free(qn);
        free(head);
        free(page);
        qn = NULL;
    }

    /* mutex lock */
    if((err = pthread_mutex_lock(&ac)) != 0) perror2("mutex lock", err);
    if(flag2){
        free(arg);
        flag2=0;
    }
    /* mutex unlock */
    if((err = pthread_mutex_unlock(&ac)) != 0) perror2("mutex unlock", err);
    printf("Thread exiting\n");
}

void perror2(const char* s, int err){
    fprintf(stderr, "%s: %s\n", s, strerror(err));
    exit(1);
}

int get_page(char** page, char* root_dir, char* buf){
    
    /* make the filename */
    char filename[120];
    strcpy(filename, root_dir);
    char* token = strtok(buf, "\n");
    char* token2 = strtok(token+4, " ");
    strcat(filename, token2);
    printf("Filename : %s\n", filename);
    
    // DIR* di = opendir("../../websites/site_0");
    // struct dirent *ent;
    // while ((ent = readdir (di)) != NULL) {
    //     printf ("%s\n", ent->d_name);
    // }

    long len;
    FILE * f;
    f = fopen (filename, "rb");

    if(f == NULL){
        if(errno == EACCES){    /* Dont't have Permisions */
            *page = malloc (100);
            strcpy(*page, "<html>Trying to access this file but don't think i can make it.</html>\0");
            return 403;
        }
        else if(errno == ENOENT){   /* Does not Exist */
            *page = malloc(100);
            strcpy(*page, "<html>Sorry dude, couldn't find this file</html>\0");
            return 404;
        }
        else{
            printf("%d", errno);
            perror2("fopen", -1);
        }
    }

    fseek (f, 0, SEEK_END);
    len = ftell (f);
    fseek (f, 0, SEEK_SET);
    *page = malloc (len+1);
    fread (*page, 1, len, f);
    bytes += len;
    pages += 1;
    fclose (f);
    return 200;
}

args* args_Init(Queue* queue, char* new_dir){
    args* ar = malloc(sizeof(args));
    ar->queue = queue;
    ar->dir = new_dir;
    return ar;
}