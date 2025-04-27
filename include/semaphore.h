#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "os.h"
#include "priority_queue.h"
#include "scheduler_interface.h"

// Max procs waiting on a resource
#define MAX_BLOCKED_Q  10

typedef struct {
    char name[16];
    int value;             // 0 = locked, 1 = free
    Node* queue;
    int queue_size;
} semaphore_t;

// One global instance per resource
extern semaphore_t sem_user_input, sem_user_output, sem_file;

// Initialize semaphores
void sem_init_all(void);

// semWait/semSignal with PID and priority-aware unblock
void sem_wait(char *name,pcb_t* pcb, Scheduler* schedule);
void sem_signal(char *name, Scheduler* schedule);

#endif // SEMAPHORE_H
