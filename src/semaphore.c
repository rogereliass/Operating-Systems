#include <string.h>
#include <stdio.h>
#include "../include/semaphore.h"
#include "../include/scheduler_interface.h"


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
        enqueue(&(sem->queue),pcb);
        dequeue(scheduler, pcb);
        
    }
}

void sem_signal(char *name,Scheduler* scheduler) {
    semaphore_t *sem = get_semaphore(name);
    if (sem->queue_size > 0) {
        pcb_t* pcb = dequeue(&(sem->queue));
        sem->queue_size--;
        // Move PCB to READY queue here
        pcb->state = READY;
        enqueue(scheduler, pcb);
    } else {
        sem->value++;
    }
}
