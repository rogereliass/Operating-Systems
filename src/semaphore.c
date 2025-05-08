#include <string.h>
#include <stdio.h>
#include "../include/priority_queue.h"
#include "../include/semaphore.h"
#include "../include/os.h"
#include "../include/gui.h"
//#include "../include/scheduler_interface.h"
//#include "../include/priority_queue.h"

// External declaration of resource_panel
extern GtkWidget *resource_panel;

#define MAX_SEMAPHORES 3

static semaphore_t semaphores[MAX_SEMAPHORES];
static int sem_count = 0;

void sem_init_all() {
    sem_count = 0;
    
    // Clear all semaphores first
    memset(semaphores, 0, sizeof(semaphores));
    
    // Resource names in fixed order
    const char *resource_names[] = {
        RESOURCE_USER_INPUT,
        RESOURCE_USER_OUTPUT,
        RESOURCE_FILE
    };
    
    // Initialize all three semaphores
    for (int i = 0; i < 3; i++) {
        // Initialize semaphore
        strcpy(semaphores[i].name, resource_names[i]);
        semaphores[i].value = 1;  // Start as free
        semaphores[i].initialized = 1;
        semaphores[i].queue = NULL;
        semaphores[i].queue_size = 0;
        semaphores[i].current_holder = -1;
        sem_count++;
    }
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
        semaphores[sem_count].initialized = 1;
        semaphores[sem_count].current_holder = -1;
        semaphores[sem_count].queue = NULL;
        return &semaphores[sem_count++];
    }
    return NULL; // Error: too many semaphores
}

void sem_wait(char *name, pcb_t* pcb, Scheduler* scheduler) {
    semaphore_t *sem = get_semaphore(name);
    if (sem->value > 0) {
        sem->value--;
        sem->current_holder = pcb->pid;
    } else {
        // Move PCB to BLOCKED queue here
        pcb->state = BLOCKED;
        update_pcb_in_memory(pcb); // Update PCB in memory
        sem->queue_size++;
        enqueue(&(sem->queue), pcb);
        scheduler->scheduler_dequeue(scheduler, pcb);
    }
    
    // Update the GUI to reflect the new resource state
    if (resource_panel) {
        update_resource_panel(resource_panel);
    }
}

void sem_signal(char *name, Scheduler* scheduler) {
    semaphore_t *sem = get_semaphore(name);
    if (sem->queue_size > 0) {
        pcb_t* pcb = dequeue(&(sem->queue));
        sem->queue_size--;
        // Move PCB to READY queue here
        pcb->state = READY;
        update_pcb_in_memory(pcb); // Update PCB in memory
        scheduler->scheduler_enqueue(scheduler, pcb);
        sem->current_holder = pcb->pid;
    } else {
        sem->value++;
        sem->current_holder = -1;
    }
    
    // Update the GUI to reflect the new resource state
    if (resource_panel) {
        update_resource_panel(resource_panel);
    }
}

void get_resource_status(resource_status_t* status_array, int* num_resources) {
    // Always return exactly 3 resources in fixed order
    *num_resources = 3;
    
    // Define our three resources in fixed order
    struct {
        const char* name;
        int index;  // -1 means not found
    } resources[] = {
        {RESOURCE_USER_INPUT, -1},
        {RESOURCE_USER_OUTPUT, -1},
        {RESOURCE_FILE, -1}
    };
    
    // First, find each resource in the semaphores array
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < sem_count; j++) {
            if (strcmp(semaphores[j].name, resources[i].name) == 0) {
                resources[i].index = j;
                break;
            }
        }
    }
    
    // Then, populate the status array in the fixed order
    for (int i = 0; i < 3; i++) {
        // Copy the resource name
        strcpy(status_array[i].name, resources[i].name);
        
        if (resources[i].index != -1) {
            // Resource was found in semaphores array
            semaphore_t* sem = &semaphores[resources[i].index];
            status_array[i].value = sem->value;
            status_array[i].current_holder = sem->current_holder;
            status_array[i].queue_size = sem->queue_size;
            
            // Copy waiting PIDs
            if (sem->queue_size > 0) {
                status_array[i].waiting_pids = malloc(sem->queue_size * sizeof(int));
                Node* current = sem->queue;
                int j = 0;
                while (current && j < sem->queue_size) {
                    status_array[i].waiting_pids[j++] = current->pcb->pid;
                    current = current->next;
                }
            } else {
                status_array[i].waiting_pids = NULL;
            }
        } else {
            // Resource not found, set default values
            status_array[i].value = 1;  // Free
            status_array[i].current_holder = -1;  // No holder
            status_array[i].queue_size = 0;
            status_array[i].waiting_pids = NULL;
        }
    }
}
