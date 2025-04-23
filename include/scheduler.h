#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "os.h"

// Choose algorithm
typedef enum { S_FCFS, S_RR, S_MLFQ } sched_algo_t;

// Init with algorithm, quantum (for RR/MLFQ)
void sched_init(sched_algo_t algo, int quantum);

// Enqueue a PCB into ready set
void sched_enqueue(pcb_t *proc);

// Pick the next PCB to run (removes from ready)
pcb_t *sched_next(void);

// Handle a quantum expiration (for RR/MLFQ)
void sched_preempt(pcb_t *proc);

#endif // SCHEDULER_H
