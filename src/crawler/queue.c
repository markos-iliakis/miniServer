#include "queue.h"
 
// A utility function to create a new linked list node.
QNode* newNode(char* k, int ret){
    QNode* temp = malloc(sizeof(QNode));
    temp->key = malloc(strlen(k)*sizeof(char)+1);
    strcpy(temp->key, k);
    temp->ret = ret;
    // temp->fd = fd;
    temp->next = NULL;
    return temp; 
}
 
// A utility function to create an empty queue
Queue* createQueue(){
    Queue *q = malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    q->counter = 0;
    return q;
}
 
// The function to add a key k to q
void enQueue(Queue* q, char* k, int ret){

    // Create a new LL node
    QNode* temp = newNode(k, ret);
 
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL){
        q->front = q->rear = temp;
        q->counter++;
        // printf("enqueue : %s counter : %d\n", temp->key, q->counter);
        return;
    }
 
    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;

    q->counter++;
    // printf("enqueue : %s counter : %d\n", temp->key, q->counter);

}
 
// Function to remove a key from given queue q
QNode* deQueue(Queue* q){

    // If queue is empty, return NULL.
    if (q->front == NULL){
        printf("queue was empty, cannot dequeue\n");
       return NULL;
    }
    // printf("dequeue :  %s counter : %d\n", q->front->key, q->counter);
    // Store previous front and move front one node ahead
    QNode* temp = newNode(q->front->key, q->front->ret);

    QNode* temp2 = q->front->next;
    free(q->front->key);
    free(q->front);
    q->front = temp2;
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
       q->rear = NULL;

    q->counter--;
    
    return temp;
}

// Function to check if queue is empty
int qEmpty(Queue* q){
    if(q->counter == 0)
        return 1;
    else
        return 0;
}

void destroyqueue(Queue* queue){
    QNode* qn = NULL;
    while(!qEmpty(queue)){
        qn = deQueue(queue);
        if(qn->key != NULL) free(qn->key);
        if(qn != NULL) free(qn);
        qn = NULL;
    }
    
}