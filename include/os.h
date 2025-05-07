#ifndef OS_H
#define OS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

#define MAX_PROCS     10
#define MAX_MEM_WORDS 60
#define MAX_VARS      3
#define MAX_LINE_LEN  128

typedef enum { NEW, READY, RUNNING, BLOCKED, TERMINATED } proc_state_t;

// A parsed instruction
typedef enum {
    INST_ASSIGN,
    INST_PRINT,
    INST_PRINT_FROM_TO,
    INST_WRITE_FILE,
    INST_READ_FILE,
    INST_SEM_WAIT,
    INST_SEM_SIGNAL
} inst_type_t;

typedef struct {
    inst_type_t type;
    char arg1[32];
    char arg2[32];   // some instrs use two args
} instruction_t;

// Process Control Block
typedef struct pcb{
    int       pid;
    proc_state_t state;
    int       priority;
    int       pc;         // index of next instruction
    int       mem_low;    // lower memory index
    int       mem_high;   // upper memory index
    int       pcb_index;  // inicates when pcb starts in memory (constant value)
    int       time_in_queue; // time spent in the queue  
    //instruction_t *code;  // pointer into loaded code array
} pcb_t;

void exec_print(pcb_t *proc, instruction_t *inst);
void exec_assign(pcb_t *proc, instruction_t *inst);
void exec_write_file(pcb_t *proc, instruction_t *inst);
void exec_read_file(pcb_t *proc, instruction_t *inst);
void exec_print_from_to(pcb_t *proc, instruction_t *inst);
char * state_type_to_string(proc_state_t state);
void update_pcb_in_memory(pcb_t *proc);
// void exec_semWait(pcb_t proc, instruction_t *inst, Scheduler* scheduler);
// void exec_semSignal(instruction_t inst, Scheduler* scheduler);

#endif
