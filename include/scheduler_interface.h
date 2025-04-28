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

    // scheduler-specific data
    void *data;
};

#endif // SCHEDULER_INTERFACE_H