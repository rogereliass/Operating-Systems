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
    fcfs_data_t *q = (fcfs_data_t*) self->data;
    q->queue[(q->head)-1] = proc;
    q->head--;
}

static void destroy_fcfs(Scheduler *self) {
    free(self->data);
    free(self);
}
static void dequeue_fcfs(Scheduler* sched, pcb_t* proc) {
    fcfs_data_t *q = (fcfs_data_t*) sched->data;
    // Only remove if it's the currently scheduled process (at the head)
    // This is called when the process blocks itself (e.g., semWait)
    if (q->head < q->tail && q->queue[q->head-1] == proc) {
         // Effectively, the 'next' call already advanced head, so we don't need to do much here.
         // If 'next' hadn't been called yet, we'd do q->head++.
    }
}

Scheduler* create_fcfs_scheduler() {
    Scheduler *s = malloc(sizeof(Scheduler));
    if (!s) return NULL;
    
    s->data = calloc(1, sizeof(fcfs_data_t));
    if (!s->data) {
        free(s);
        return NULL;
    }
    
    s->scheduler_enqueue = enqueue_fcfs;
    s->next = next_fcfs;
    s->preempt = preempt_fcfs;
    s->destroy = destroy_fcfs;
    s->scheduler_dequeue = dequeue_fcfs;  
    
    return s;
}
