#ifndef SCHEDULER_INTERFACE_H
#define SCHEDULER_INTERFACE_H

#include "os.h"

typedef struct Scheduler Scheduler;

struct Scheduler {
    void (*scheduler_enqueue)(Scheduler *self, pcb_t *proc);
    pcb_t* (*next)(Scheduler *self);
    void (*preempt)(Scheduler *self, pcb_t *proc);
    void (*destroy)(Scheduler *self);  // cleanup if needed
    void (*scheduler_dequeue)(Scheduler *self, pcb_t *proc);
    pcb_t* (*queue)(Scheduler *self);  // get the next process in the queue
    int (*queue_size)(Scheduler *self); // get the size of the queue
    int (*queue_empty)(Scheduler *self); // check if the queue is empty

    // scheduler-specific data
    void *data;
};

#endif // SCHEDULER_INTERFACE_H