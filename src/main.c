#include "os.h"
#include "memory.h"
#include "parser.h"
#include "scheduler.h"
#include "semaphore.h"

int main(int argc, char **argv) {
    // 1) Init
    mem_init();
    sem_init_all();
    sched_init(S_FCFS, /* default */ 1);

    // 2) Load programs (hard‐coded or via argv)
    instruction_t *code1;
    int n1 = parse_program("programs/Program_1.txt", &code1);

    instruction_t *code2;
    int n2 = parse_program("programs/Program_2.txt", &code2);

    instruction_t *code3;
    int n3 = parse_program("programs/Program_3.txt", &code3);

    // 3) Create PCBs, alloc memory, enqueue
    pcb_t procs[3];
    for (int i = 0; i < 3; i++) {
        pcb_t *p = &procs[i];
        p->pid = i+1;
        p->state = READY;
        p->priority = 0;
        p->pc = 0;
        // alloc memory: code + vars + PCB size estimate
        int blk = mem_alloc( /* estimate words */ 10 );
        p->mem_low  = blk;
        p->mem_high = blk + 9;
        p->code = (i==0? code1 : /* ... */);
        sched_enqueue(p);
    }

    // 4) Simulation loop
    int clock = 0;
    while (true) {
        pcb_t *cur = sched_next();
        if (!cur) break;  // done
        cur->state = RUNNING;
        // fetch instruction
        instruction_t *inst = &cur->code[cur->pc++];
        // dispatch by inst->type: call functions that do assign/print/sem...
        // each call takes “1 clock cycle” → clock++

        // after executing:
        if (/* cur still READY or BLOCKED */)
           sched_preempt(cur);

        clock++;
    }
    return 0;
}
