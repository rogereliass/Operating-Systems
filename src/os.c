#include "os.h"
#include "memory.h"

void exec_write_file(pcb_t *proc, instruction_t *inst) {
    FILE *file = fopen(inst->arg1, "w");
    if (!file) {
        return -1;
    }
    fprintf(file, "%s", inst->arg2); // Writing the content to the file
    fclose(file);
}

void exec_write_file(pcb_t *proc, instruction_t *inst) {
    FILE *file = fopen(inst->arg1, "w");
    if (!file) {
        return -1;
    }
    fprintf(file, "%s", inst->arg2); // Writing the content to the file
    fclose(file);
}

void exec_read_file(pcb_t *proc, instruction_t *inst) {
    FILE *file = fopen(inst->arg1, "r");
    if (!file) {
        return -1;
    }
    // Read content into memory, or process it accordingly.
    fclose(file);
}