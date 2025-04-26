#include "../include/mlfq_scheduler.h"
#include <stdlib.h>
#include <stdio.h>

#define NUM_QUEUES 4

typedef struct {
    pcb_t* procs[100]; // Adjust size if needed
    int front, rear;
} queue_t;

typedef struct {
    queue_t queues[NUM_QUEUES];
    int quantums[NUM_QUEUES];
} mlfq_data_t;

// Helper Functions
void queue_init(queue_t* q) {
    q->front = q->rear = 0;
}

int queue_empty(queue_t* q) {
    return q->front == q->rear;
}

void queue_push(queue_t* q, pcb_t* proc) {
    q->procs[q->rear++] = proc;
}

pcb_t* queue_pop(queue_t* q) {
    if (queue_empty(q)) return NULL;
    return q->procs[q->front++];
}

// MLFQ Methods
void mlfq_enqueue(Scheduler* sched, pcb_t* proc) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;
    int level = proc->priority;
    if (level < 0 || level >= NUM_QUEUES) level = NUM_QUEUES - 1;
    queue_push(&data->queues[level], proc);
}

pcb_t* mlfq_next(Scheduler* sched) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;
    for (int i = 0; i < NUM_QUEUES; ++i) {
        if (!queue_empty(&data->queues[i])) {
            return queue_pop(&data->queues[i]);
        }
    }
    return NULL;
}

void mlfq_preempt(Scheduler* sched, pcb_t* proc) {
    if (proc->priority < NUM_QUEUES - 1) {
        proc->priority += 1; // Demote priority
    }
    mlfq_enqueue(sched, proc);
}

void mlfq_destroy(Scheduler* sched) {
    if (sched->data) free(sched->data);
    free(sched);
}

Scheduler* create_mlfq_scheduler() {
    Scheduler* sched = (Scheduler*)malloc(sizeof(Scheduler));
    mlfq_data_t* data = (mlfq_data_t*)malloc(sizeof(mlfq_data_t));

    for (int i = 0; i < NUM_QUEUES; ++i) {
        queue_init(&data->queues[i]);
        data->quantums[i] = 1 << i; // 1, 2, 4, 8
    }

    sched->enqueue = mlfq_enqueue;
    sched->next = mlfq_next;
    sched->preempt = mlfq_preempt;
    sched->destroy = mlfq_destroy;
    sched->data = data;

    return sched;
}
