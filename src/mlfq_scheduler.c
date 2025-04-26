// src/mlfq_scheduler.c
#include <stdlib.h>
#include "../include/mlfq_scheduler.h"

#define NUM_QUEUES 4
#define MAX_QUEUE 100

typedef struct {
    pcb_t* queue[MAX_QUEUE];
    int head, tail;
    int quantum;     // quantum for this level
    int ticks_used;  // ticks for current running process
    pcb_t* current;  // currently running process
} mlfq_queue_t;

typedef struct {
    mlfq_queue_t levels[NUM_QUEUES];
} mlfq_data_t;

// Helper Functions
static void queue_init(mlfq_queue_t* q, int quantum) {
    q->head = q->tail = 0;
    q->quantum = quantum;
    q->ticks_used = 0;
    q->current = NULL;
}

static int queue_empty(mlfq_queue_t* q) {
    return q->head == q->tail;
}

static void queue_push(mlfq_queue_t* q, pcb_t* proc) {
    q->queue[q->tail++] = proc;
}

static pcb_t* queue_pop(mlfq_queue_t* q) {
    if (queue_empty(q)) return NULL;
    return q->queue[q->head++];
}

static void queue_remove(mlfq_queue_t* q, pcb_t* proc) {
    int new_tail = q->head;
    for (int i = q->head; i < q->tail; i++) {
        if (q->queue[i] != proc) {
            q->queue[new_tail++] = q->queue[i];
        }
    }
    q->tail = new_tail;
}

// MLFQ Methods
static void mlfq_enqueue(Scheduler* sched, pcb_t* proc) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;
    int level = proc->priority;
    if (level < 0 || level >= NUM_QUEUES) level = NUM_QUEUES - 1;
    queue_push(&data->levels[level], proc);
}

static pcb_t* mlfq_next(Scheduler* sched) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;

    for (int i = 0; i < NUM_QUEUES; ++i) {
        mlfq_queue_t* q = &data->levels[i];
        if (!queue_empty(q)) {
            q->current = queue_pop(q);
            q->ticks_used = 0;
            return q->current;
        }
    }
    return NULL;
}

static void mlfq_preempt(Scheduler* sched, pcb_t* proc) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;
    int level = proc->priority;
    if (level < 0 || level >= NUM_QUEUES) level = NUM_QUEUES - 1;
    
    mlfq_queue_t* q = &data->levels[level];

    q->ticks_used++;

    if (q->ticks_used >= q->quantum) {
        // Process used up quantum, demote if possible
        if (proc->priority < NUM_QUEUES - 1) {
            proc->priority++;
        }
        queue_push(&data->levels[proc->priority], proc);
        q->current = NULL;
        q->ticks_used = 0;
    } else {
        // Let it continue
        q->current = proc;
    }
}

static void mlfq_destroy(Scheduler* sched) {
    if (sched->data) free(sched->data);
    free(sched);
}

static pcb_t* dequeue_mlfq(Scheduler* sched, pcb_t* proc) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;
    for (int i = 0; i < NUM_QUEUES; ++i) {
        queue_remove(&data->levels[i], proc);
    }
    return proc;
}

Scheduler* create_mlfq_scheduler() {
    Scheduler* sched = (Scheduler*) malloc(sizeof(Scheduler));
    mlfq_data_t* data = (mlfq_data_t*) malloc(sizeof(mlfq_data_t));

    for (int i = 0; i < NUM_QUEUES; ++i) {
        queue_init(&data->levels[i], 1 << i); // quantum: 1, 2, 4, 8
    }

    sched->enqueue = mlfq_enqueue;
    sched->next    = mlfq_next;
    sched->preempt = mlfq_preempt;
    sched->destroy = mlfq_destroy;
    sched->dequeue = dequeue_mlfq;
    sched->data    = data;

    return sched;
}
