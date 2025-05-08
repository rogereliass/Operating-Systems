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
    proc->state = READY;
    proc->time_in_queue = 0; // Reset time in queue
    update_pcb_in_memory(proc); // Update PCB in memory
}

static pcb_t* next_rr(Scheduler *self) {
    rr_data_t *rr = (rr_data_t*) self->data;

    if(rr->current && rr->current->state != TERMINATED) {
        // Increment time for all processes in the queue
        for (int i = rr->head; i < rr->tail; i++) {
            if (rr->queue[i] != rr->current) {
                rr->queue[i]->time_in_queue++;
                update_pcb_in_memory(rr->queue[i]); // Update PCB in memory
            }
        }
        // Update the time for the current process
        rr->current->time_in_queue = 0; // Reset time in queue for the running process
        update_pcb_in_memory(rr->current); // Update PCB in memory
        // If current process is still running, return it
        return rr->current;
    }
    if (rr->head < rr->tail) {
        rr->current = rr->queue[rr->head++];
        rr->ticks_used = 0;
        for (int i = rr->head; i < rr->tail; i++) {
            if (rr->queue[i] != rr->current) {
                rr->queue[i]->time_in_queue++;
                update_pcb_in_memory(rr->queue[i]); // Update PCB in memory
            }
        }
        rr->current->time_in_queue = 0; // Reset time in queue for the running process
        update_pcb_in_memory(rr->current); // Update PCB in memory
        return rr->current;
    }

    return NULL;  // no process to run
}

static void preempt_rr(Scheduler *self, pcb_t *proc) {
    rr_data_t *rr = (rr_data_t*) self->data;

    rr->ticks_used++;

    if (rr->ticks_used >= rr->quantum) {
        // Time's up — rotate process
        proc->state = READY;
        update_pcb_in_memory(proc); // Update PCB in memory
        enqueue_rr(self, proc);
        rr->current = NULL;
        rr->ticks_used = 0;
    } else {
        // Let it continue
        rr->current = proc;
        //rr->queue[(rr->head)-1] = proc;
        //rr->head--;
    }
}

static void destroy_rr(Scheduler *self) {
    free(self->data);
    free(self);
}

static void dequeue_rr(Scheduler* sched, pcb_t* proc) {
    rr_data_t* rr = (rr_data_t*) sched->data;

    proc->time_in_queue = 0; // Reset time in queue
    update_pcb_in_memory(proc); // Update PCB in memory
    
    // int new_tail = rr->head;

    // for (int i = rr->head; i < rr->tail; i++) {
    //     if (rr->queue[i] != proc) {
    //         rr->queue[new_tail++] = rr->queue[i];
    //     }
    // }

    // rr->tail = new_tail;
    rr->current = NULL;
    //rr->ticks_used = 0;
}
static pcb_t* queue_rr(Scheduler *self) {
    rr_data_t *rr = (rr_data_t*) self->data;
    if (rr->head < rr->tail)
        return rr->queue[rr->head];
    return NULL;
}
static int queue_size_rr(Scheduler *self) {
    rr_data_t *rr = (rr_data_t*) self->data;
    return rr->tail - rr->head;
}
static int queue_empty_rr(Scheduler *self) {
    rr_data_t *rr = (rr_data_t*) self->data;
    return rr->head == rr->tail;
}

Scheduler* create_rr_scheduler(int quantum) {
    Scheduler *s = malloc(sizeof(Scheduler));
    rr_data_t *rr = calloc(1, sizeof(rr_data_t));

    rr->quantum = quantum;
    rr->head = rr->tail = 0;
    s->type = SCHEDULER_RR;
    s->scheduler_enqueue = enqueue_rr;
    s->next    = next_rr;
    s->preempt = preempt_rr;
    s->destroy = destroy_rr;
    s->scheduler_dequeue = dequeue_rr;
    s->queue     = queue_rr;
    s->queue_size = queue_size_rr;
    s->queue_empty = queue_empty_rr;
    s->data    = rr;

    return s;
}
