#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "os.h"
#include "priority_queue.h"
#include "scheduler_interface.h"

// Max procs waiting on a resource
#define MAX_BLOCKED_Q  10

// Resource names
#define RESOURCE_USER_INPUT  "userInput"
#define RESOURCE_USER_OUTPUT "userOutput"
#define RESOURCE_FILE       "file"

typedef struct {
    char name[16];
    int value;             // 0 = locked, 1 = free
    Node* queue;
    int queue_size;
    int initialized;       // Flag to indicate if semaphore is initialized
    int current_holder;    // PID of process currently holding the resource (-1 if none)
} semaphore_t;

// Structure to hold resource status information for GUI
typedef struct {
    char name[16];
    int value;
    int queue_size;
    int* waiting_pids;     // Array of PIDs waiting for this resource
    int current_holder;    // PID of process currently holding the resource (-1 if none)
} resource_status_t;

// Initialize semaphores
void sem_init_all(void);

// semWait/semSignal with PID and priority-aware unblock
void sem_wait(char *name, pcb_t* pcb, Scheduler* schedule);
void sem_signal(char *name, Scheduler* schedule);

// Get status of all resources for GUI display
void get_resource_status(resource_status_t* status_array, int* num_resources);

#endif // SEMAPHORE_H
