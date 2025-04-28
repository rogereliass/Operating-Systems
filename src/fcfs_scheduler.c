#include <stdlib.h>
#include "../include/fcfs_scheduler.h"

#define MAX_QUEUE 100

typedef struct {
    pcb_t* queue[MAX_QUEUE];
    int head, tail;
} fcfs_data_t;

static void enqueue_fcfs(Scheduler *self, pcb_t *proc) {
    fcfs_data_t *q = (fcfs_data_t*) self->data;
    q->queue[q->tail++] = proc;
}

static pcb_t* next_fcfs(Scheduler *self) {
    fcfs_data_t *q = (fcfs_data_t*) self->data;
    if (q->head < q->tail)
        return q->queue[q->head++];
    return NULL;
}

static void preempt_fcfs(Scheduler *self, pcb_t *proc) {
}

static void destroy_fcfs(Scheduler *self) {
    free(self->data);
    free(self);
}
static void dequeue_fcfs(Scheduler* sched, pcb_t* proc) {
    // fcfs_data_t* data = (fcfs_data_t*) sched->data;
    // if (data->front == data->rear) return NULL;
    // return data->queue[data->front++]; 
}

Scheduler* create_fcfs_scheduler() {
    Scheduler *s = malloc(sizeof(Scheduler));
    s->scheduler_enqueue = enqueue_fcfs;
    s->next = next_fcfs;
    s->preempt = preempt_fcfs;
    s->destroy = destroy_fcfs;
    s->scheduler_dequeue = dequeue_fcfs;  
    s->data = calloc(1, sizeof(fcfs_data_t));
    return s;
}
