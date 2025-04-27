#ifndef OS_H
#define OS_H

#include <stdint.h>
#include "semaphore.h"

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
    instruction_t *code;  // pointer into loaded code array
} pcb_t;

#endif
