#pragma once
/**
* CSC369 Assignment 2
*/
#include "car.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct _QueueNode {
    int id;
    struct _QueueNode *next;
} QueueNode;

typedef struct _Queue {
    QueueNode *front;
    QueueNode *back;
} Queue;

void initQueueNode(QueueNode *qn, int id);

void initQueue(Queue *q);

void enterQueue(Car* car, Queue *q);

void exitQueue(Queue *q);

void destroyQueue(Queue *q);

int first(Car *car, Queue *q);