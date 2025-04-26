// src/round_robin_scheduler.c
#include <stdlib.h>
#include "../include/round_robin_scheduler.h"

#define MAX_QUEUE 100

typedef struct {
    pcb_t* queue[MAX_QUEUE];
    int head, tail;
    int quantum;     // user-defined time slice
    int ticks_used;  // counts ticks by current process
    pcb_t* current;  // currently running process
} rr_data_t;

static void enqueue_rr(Scheduler *self, pcb_t *proc) {
    rr_data_t *rr = (rr_data_t*) self->data;
    rr->queue[rr->tail++] = proc;
}

static pcb_t* next_rr(Scheduler *self) {
    rr_data_t *rr = (rr_data_t*) self->data;

    if (rr->head < rr->tail) {
        rr->current = rr->queue[rr->head++];
        rr->ticks_used = 0;
        return rr->current;
    }

    return NULL;  // no process to run
}

static void preempt_rr(Scheduler *self, pcb_t *proc) {
    rr_data_t *rr = (rr_data_t*) self->data;

    rr->ticks_used++;

    if (rr->ticks_used >= rr->quantum) {
        // Time's up — rotate process
        enqueue_rr(self, proc);
        rr->current = NULL;
        rr->ticks_used = 0;
    } else {
        // Let it continue
        rr->current = proc;
    }
}

static void destroy_rr(Scheduler *self) {
    free(self->data);
    free(self);
}

Scheduler* create_rr_scheduler(int quantum) {
    Scheduler *s = malloc(sizeof(Scheduler));
    rr_data_t *rr = calloc(1, sizeof(rr_data_t));

    rr->quantum = quantum;
    rr->head = rr->tail = 0;

    s->enqueue = enqueue_rr;
    s->next    = next_rr;
    s->preempt = preempt_rr;
    s->destroy = destroy_rr;
    s->data    = rr;

    return s;
}
