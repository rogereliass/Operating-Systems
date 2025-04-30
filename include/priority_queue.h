#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os.h"

typedef struct pcb pcb_t;

typedef struct Node {
    pcb_t* pcb;
    struct Node* next;
} Node;

Node* createNode(pcb_t* pcb);
void enqueue(Node** head, pcb_t* pcb);
pcb_t* dequeue(Node** head);
int isEmpty(Node* head);

#endif