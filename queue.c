/**
* CSC369 Assignment 2
*/
#include "myCommon.h"
#include "common.h"

// ===========
// Queue Helper Functions

void initQueueNode(QueueNode *qn, int id) {
    qn->id = id;
    qn->next = NULL;
}

void initQueue(Queue *q) {
    q->front = NULL;
    q->back = NULL;
}

void enterQueue(Car* car, Queue *q) {
    QueueNode *qn = (QueueNode*)malloc(sizeof(QueueNode));
    initQueueNode(qn, car->index);
    if (q->front == NULL) {
        q->front = qn;
        q->back = qn;
    } else {
        q->back->next = qn;
        q->back = qn;
    }
}

void exitQueue(Queue *q) {
    QueueNode *front = q->front;
    if (front == NULL) {
        return;
    }
    q->front = q->front->next;
    free(front);
}

void destroyQueue(Queue *q) {
    while (q->front != NULL) {
        exitQueue(q);
    }
}

int first(Car *car, Queue *q) {
    return car->index == q->front->id;
}
// ====================================

// ===========
// Synchonization Error Checking Functions

void lock(pthread_mutex_t *mutex) {
    int returnValue = pthread_mutex_lock(mutex);
    if (returnValue != 0) {
        perror("Locking mutex failed."
                "@ " __FILE__ " : " LINE_STRING "\n");  
    }
}

void destroyMutex(pthread_mutex_t *mutex) {
    int returnValue = pthread_mutex_destroy(mutex);
    if (returnValue != 0) {
        perror("Destroying mutex failed."
                "@ " __FILE__ " : " LINE_STRING "\n");  
    }
}

void waitCV(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    int returnValue = pthread_cond_wait(cond, mutex);
    if (returnValue != 0) {
        perror("Waiting on condition variable failed."
                "@ " __FILE__ " : " LINE_STRING "\n");  
    }
}

void broadcastCV(pthread_cond_t *cond) {
    int returnValue = pthread_cond_broadcast(cond);
    if (returnValue != 0) {
        perror("Broadcasting to condition variable failed."
                "@ " __FILE__ " : " LINE_STRING "\n");  
    }
}

void destroyCV(pthread_cond_t *cond) {
    int returnValue = pthread_cond_destroy(cond);
    if (returnValue != 0) {
        perror("Destroying condition variable failed."
                "@ " __FILE__ " : " LINE_STRING "\n");  
    }
}
// ====================================