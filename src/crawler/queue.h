#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A linked list (LL) node to store a queue entry
typedef struct QNode{
    char* key;
    int ret;
    // int fd;
    struct QNode* next;
}QNode;
 
// The queue, front stores the front node of LL and rear stores the
// last node of LL
typedef struct Queue{
    QNode* front;
    QNode* rear;
    int counter;
}Queue;

QNode* newNode(char* k, int ret);
Queue* createQueue();
void enQueue(Queue* q, char* k, int ret);
QNode* deQueue(Queue *q);
int qEmpty(Queue* q);
void destroyqueue(Queue* queue);