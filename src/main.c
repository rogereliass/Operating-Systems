#include "../include/os.h"
#include "../include/memory.h"
#include "../include/parser.h"
#include "../include/semaphore.h"
#include "../include/scheduler_interface.h"
#include "../include/fcfs_scheduler.h"
#include "../include/round_robin_scheduler.h"
#include "../include/mlfq_scheduler.h"
#include "memory.c"
#include "os.c"
#include "parser.c"
#include "semaphore.c"
#include "fcfs_scheduler.c"
#include "round_robin_scheduler.c"
#include "mlfq_scheduler.c"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Global Variables
Scheduler* scheduler = NULL;
pcb_t processes[3];
int num_processes = 0;
int clock_tick = 0;
int simulation_running = 0; // 0 = stopped, 1 = running
int auto_mode = 0;           // 0 = step-by-step, 1 = auto-run

// Function Prototypes
void load_programs();
void choose_scheduler();
void simulation_step();
//void update_gui();

void choose_scheduler() {
    //should be from gui
    printf("Choose scheduler:\n1. FCFS\n2. RR\n3. MLFQ\nChoice: ");
    int choice, quantum;
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            scheduler = create_fcfs_scheduler();
            break;
        case 2:
            printf("Enter quantum: ");
            scanf("%d", &quantum);
            scheduler = create_rr_scheduler(quantum);
            break;
        case 3:
            scheduler = create_mlfq_scheduler();
            break;
        default:
            printf("Invalid choice.\n");
            exit(1);
    }
}
void load_programs() {
    const char* filenames[] = {
        "programs/Program_1.txt",
        "programs/Program_2.txt",
        "programs/Program_3.txt"
    };

    num_processes = 3;

    for (int i = 0; i < num_processes; i++) {
        FILE* f = fopen(filenames[i], "r");
        if (!f) {
            printf("Failed to open %s\n", filenames[i]);
            exit(1);
        }

        // Estimate number of instructions first
        int code_start = mem_alloc(20); // allocate big enough block: code + 3 vars
        if (code_start < 0) {
            printf("Memory allocation failed for process %d\n", i + 1);
            exit(1);
        }

        int var_start = code_start;         // variables will be at the start
        int code_mem_start = code_start + 3; // instructions start after vars

        int current_idx = code_mem_start;
        char line[MAX_LINE_LEN];

        int instruction_count = 0;
        while (fgets(line, sizeof(line), f)) {
            if (current_idx >= MAX_MEM_WORDS) {
                printf("Memory overflow loading %s\n", filenames[i]);
                exit(1);
            }

            // Save each instruction as text inside memory
            mem_write(current_idx, "instruction", line); 
            current_idx++;
            instruction_count++;
        }
        fclose(f);

        // Setup PCB
        processes[i].pid = i + 1;
        processes[i].state = READY;
        processes[i].priority = 0; // MLFQ start at top level
        processes[i].pc = code_mem_start;
        processes[i].mem_low = var_start;
        processes[i].mem_high = current_idx - 1;

        scheduler->enqueue(scheduler, &processes[i]);
    }
}
void simulation_step() {
    clock_tick++;

    pcb_t* current = scheduler->next(scheduler);
    if (!current) {
        printf("All processes finished.\n");
        simulation_running = 0;
        return;
    }

    current->state = RUNNING;

    // Fetch and execute one instruction
    instruction_t* inst = &current->code[current->pc];

    // Dispatch based on instruction type
    switch (inst->type) {
        case INST_ASSIGN: exec_assign(current, inst); break;
        case INST_PRINT: exec_print(current, inst); break;
        case INST_PRINT_FROM_TO: exec_print_from_to(current, inst); break;
        case INST_WRITE_FILE: exec_write_file(current, inst); break;
        case INST_READ_FILE: exec_read_file(current, inst); break;
        case INST_SEM_WAIT: exec_semWait(current, inst,scheduler); break;
        case INST_SEM_SIGNAL: exec_semSignal(inst,scheduler); break;
    }

    current->pc++;

    // Check if process finished
    if ( current->state == BLOCKED) {
        return;
        // memory free if needed
    }
    else if (current->pc >= current->mem_high){
        current->state = TERMINATED;
    }
    else {
        scheduler->preempt(scheduler, current); // If not terminated or blocked
    }
}


int main() {
    // Initialize subsystems
    mem_init();
    sem_init_all();

    choose_scheduler();   // Choose FCFS / RR / MLFQ
    load_programs();      // Load programs into PCBs and memory

    bool exit_program = false; // new flag to control when to leave the loop

    while (!exit_program) {
        // 1. Refresh GUI
        //update_gui(); 

        // 2. Check Simulation State
        if (simulation_running) {
            simulation_step(); // one clock tick
            if (!auto_mode) simulation_running = false; // in step mode, stop after one step
        }

        // 3. Handle User Input
        //int user_action = get_user_action(); // hypothetical function: returns Start/Step/Stop/Reset/Exit
        // int user_action=scanf

        // switch (user_action) {
        //     case ACTION_START:
        //         simulation_running = true;
        //         auto_mode = true;
        //         break;

        //     case ACTION_STEP:
        //         simulation_running = true;
        //         auto_mode = false;
        //         break;

        //     case ACTION_STOP:
        //         simulation_running = false;
        //         break;

        //     case ACTION_RESET:
        //         // Cleanup and Reload Everything
        //         scheduler->destroy(scheduler);
        //         mem_init();
        //         sem_init_all();
        //         choose_scheduler();
        //         load_programs();
        //         clock_tick = 0;
        //         simulation_running = false;
        //         auto_mode = false;
        //         break;

        //     case ACTION_EXIT:
        //         // Cleanup and Exit Program
        //         scheduler->destroy(scheduler);
        //         exit_program = true;
        //         break;
        // }
    }

    return 0;
}