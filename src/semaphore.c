#include <string.h>
#include <stdio.h>
#include "../include/priority_queue.h"
#include "../include/semaphore.h"
//#include "../include/scheduler_interface.h"
//#include "../include/priority_queue.h"



#define MAX_SEMAPHORES 3

static semaphore_t semaphores[MAX_SEMAPHORES];
static int sem_count = 0;

void sem_init_all() {
    sem_count = 0;
}

semaphore_t* get_semaphore(char *name) {
    for (int i = 0; i < sem_count; i++) {
        if (strcmp(semaphores[i].name, name) == 0)
            return &semaphores[i];
    }
    if (sem_count < MAX_SEMAPHORES) {
        strcpy(semaphores[sem_count].name, name);
        semaphores[sem_count].value = 1;
        semaphores[sem_count].queue_size = 0;
        return &semaphores[sem_count++];
    }
    return NULL; // Error: too many semaphores
}

void sem_wait(char *name,pcb_t* pcb, Scheduler* scheduler ) {
    semaphore_t *sem = get_semaphore(name);
    if (sem->value > 0) {
        sem->value--;
    } else {
        // Move PCB to BLOCKED queue here
        pcb->state = BLOCKED;
        update_pcb_in_memory(pcb); // Update PCB in memory
        sem->queue_size++;
        enqueue(&(sem->queue),pcb);
        scheduler->scheduler_dequeue(scheduler, pcb);
        
    }
}

void sem_signal(char *name,Scheduler* scheduler) {
    semaphore_t *sem = get_semaphore(name);
    if (sem->queue_size > 0) {
        pcb_t* pcb = dequeue(&(sem->queue));
        sem->queue_size--;
        // Move PCB to READY queue here
        pcb->state = READY;
        update_pcb_in_memory(pcb); // Update PCB in memory
        scheduler->scheduler_enqueue(scheduler, pcb);
    } else {
        sem->value++;
    }
}

void get_resource_status(resource_status_t* status_array, int* num_resources) {
    *num_resources = sem_count;
    
    // For each semaphore in the system
    for (int i = 0; i < sem_count; i++) {
        semaphore_t* sem = &semaphores[i];
        
        // Copy basic information
        strcpy(status_array[i].name, sem->name);
        status_array[i].value = sem->value;
        status_array[i].queue_size = sem->queue_size;
        status_array[i].current_holder = (sem->value == 0) ? 1 : -1; // Simplified: if locked, assume PID 1 holds it
        
        // Get list of waiting PIDs
        if (sem->queue_size > 0) {
            status_array[i].waiting_pids = malloc(sem->queue_size * sizeof(int));
            Node* current = sem->queue;
            int j = 0;
            while (current != NULL) {
                status_array[i].waiting_pids[j++] = ((pcb_t*)current->data)->pid;
                current = current->next;
            }
        } else {
            status_array[i].waiting_pids = NULL;
        }
    }
}
