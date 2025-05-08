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
    if (level < 0 || level >= NUM_QUEUES) {
        level = NUM_QUEUES - 1;
        proc->priority = level;
        update_pcb_in_memory(proc); // Update PCB in memory
    }
    proc->time_in_queue = 0; // Reset time in queue
    update_pcb_in_memory(proc); // Update PCB in memory
    queue_push(&data->levels[level], proc);
}

static pcb_t* mlfq_next(Scheduler* sched) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;

    for (int i = 0; i < NUM_QUEUES; ++i){
        mlfq_queue_t* q = &data->levels[i];
        if (!queue_empty(q)) {
            for (int j = q->head; j < q->tail; j++) {
                q->queue[j]->time_in_queue++;
                update_pcb_in_memory(q->queue[j]); // Update PCB in memory
            }
        }
    }
    
    // Check if any process is currently running
    for (int i = 0; i < NUM_QUEUES; ++i) {
        mlfq_queue_t* q = &data->levels[i];
        if(q->current && q->current->state == RUNNING) {
            // If current process is still running, return it
            q->current->time_in_queue = 0; // Reset time in queue for the running process
            update_pcb_in_memory(q->current); // Update PCB in memory
            return q->current;
        }
    }

    for (int i = 0; i < NUM_QUEUES; ++i) {
        mlfq_queue_t* q = &data->levels[i];
        if (!queue_empty(q)) {
            q->current = queue_pop(q);
            q->ticks_used = 0;
            q->current->time_in_queue = 0; // Reset time in queue for the running process
            update_pcb_in_memory(q->current); // Update PCB in memory
            return q->current;
        }
    }
    return NULL;
}

static void mlfq_preempt(Scheduler* sched, pcb_t* proc) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;
    int level = proc->priority;
    if (level < 0 || level >= NUM_QUEUES) {
        level = NUM_QUEUES - 1;
        proc->priority = level;
        update_pcb_in_memory(proc); // Update PCB in memory
    }
    mlfq_queue_t* q = &data->levels[level];

    q->ticks_used++;

    if (q->ticks_used >= q->quantum) {
        // Process used up quantum, demote if possible
        if (proc->priority < NUM_QUEUES - 1) {
            proc->priority++;
            update_pcb_in_memory(proc); // Update PCB in memory
        }
        proc->state = READY;
        update_pcb_in_memory(proc); // Update PCB in memory
        queue_push(&data->levels[proc->priority], proc);
        q->current = NULL;
        q->ticks_used = 0;
    } else {
        // Let it continue
        q->current = proc;
        // proc->state = READY;
        // update_pcb_in_memory(proc); // Update PCB in memory
    }
    
}

static void mlfq_destroy(Scheduler* sched) {
    if (sched->data) free(sched->data);
    free(sched);
}

static void dequeue_mlfq(Scheduler* sched, pcb_t* proc) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;
    mlfq_queue_t* q = &data->levels[proc->priority];
    // for (int i = 0; i < NUM_QUEUES; ++i) {
    //     queue_remove(&data->levels[i], proc);
    // }
    proc->time_in_queue = 0; // Reset time in queue
    update_pcb_in_memory(proc); // Update PCB in memory
    q->ticks_used++;
    if(q->ticks_used >= q->quantum){
        if (proc->priority < NUM_QUEUES - 1){
            proc->priority++;
            update_pcb_in_memory(proc); // Update PCB in memory
        }
       }
   q->current = NULL;
   q->ticks_used = 0;
}
static pcb_t* queue_mlfq(Scheduler* sched) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;
    pcb_t* queue[3];
    int j=0;
    for (int i = 0; i < NUM_QUEUES; i++) {
        if (!queue_empty(&data->levels[i])) {
            queue[j++] = data->levels[i].queue[data->levels[i].head];
        }
    }
    if (j == 0) return NULL;
    return queue[0];
}
static int queue_size_mlfq(Scheduler* sched) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;
    int size = 0;
    for (int i = 0; i < NUM_QUEUES; i++) {
        size += data->levels[i].tail - data->levels[i].head;
    }
    return size;
}
static int queue_empty_mlfq(Scheduler* sched) {
    mlfq_data_t* data = (mlfq_data_t*) sched->data;
    for (int i = 0; i < NUM_QUEUES; i++) {
        if (!queue_empty(&data->levels[i])) {
            return 0;
        }
    }
    return 1;
}

Scheduler* create_mlfq_scheduler() {
    Scheduler* sched = (Scheduler*) malloc(sizeof(Scheduler));
    mlfq_data_t* data = (mlfq_data_t*) malloc(sizeof(mlfq_data_t));

    for (int i = 0; i < NUM_QUEUES; ++i) {
        queue_init(&data->levels[i], 1 << i); // quantum: 1, 2, 4, 8
    }

    sched->type = SCHEDULER_MLFQ;
    sched->scheduler_enqueue = mlfq_enqueue;
    sched->next    = mlfq_next;
    sched->preempt = mlfq_preempt;
    sched->destroy = mlfq_destroy;
    sched->scheduler_dequeue = dequeue_mlfq;
    sched->queue = queue_mlfq;
    sched->queue_size = queue_size_mlfq;
    sched->queue_empty = queue_empty_mlfq;
    sched->data    = data;

    return sched;
}
