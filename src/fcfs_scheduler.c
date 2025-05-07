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
    proc->state = READY;
    proc->time_in_queue = 0; // Reset time in queue
    update_pcb_in_memory(proc); // Update PCB in memory
}

static pcb_t* next_fcfs(Scheduler *self) {
    fcfs_data_t *q = (fcfs_data_t*) self->data;
    if (q->head < q->tail){
        // Increment time for all processes in the queue
        for (int i = q->head + 1; i < q->tail; i++) {
            q->queue[i]->time_in_queue++;
            update_pcb_in_memory(q->queue[i]); // Update PCB in memory
        }
        // Update the time for the process at the head
        q->queue[q->head]->time_in_queue= 0; // Reset time in queue for the running process
        update_pcb_in_memory(q->queue[q->head]); // Update PCB in memory
        return q->queue[q->head++];
    }
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
static pcb_t* queue_fcfs(Scheduler *self) {
    fcfs_data_t *q = (fcfs_data_t*) self->data;
    if (q->head < q->tail)
        return q->queue[q->head];
    return NULL;
}
static int queue_size_fcfs(Scheduler *self) {
    fcfs_data_t *q = (fcfs_data_t*) self->data;
    return q->tail - q->head;
}
static int queue_empty_fcfs(Scheduler *self) {
    fcfs_data_t *q = (fcfs_data_t*) self->data;
    return q->head == q->tail;
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
    s->queue = queue_fcfs;
    s->queue_size = queue_size_fcfs;
    s->queue_empty= queue_empty_fcfs;
    
    return s;
}
