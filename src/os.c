#include "../include/os.h"
// #include "../include/memory.h"
// #include "../include/semaphore.h"
// #include "../include/scheduler_interface.h"

void exec_print(pcb_t *proc, instruction_t *inst){
    printf("%s",inst->arg1);
}

void exec_assign(pcb_t *proc, instruction_t *inst){
    char value_buffer[100];
    if (strcmp(inst->arg2, "input") == 0) {
        // Case 1: Input from user
        printf("Please enter a value for process %d: ", proc->pid);
        fgets(value_buffer, sizeof value_buffer, stdin);
        value_buffer[strcspn(value_buffer, "\n")] = '\0';  // remove newline
    }
    else if(strncmp(inst->arg2, "readFile", 8) == 0){
        // Case 2: Nested instruction: readFile a
        char *filename_var = strtok(inst->arg2 + 9, " \n");  // get the next token after "readFile"
        const char *filename = mem_read(proc->mem_low, proc->mem_high, filename_var);
        if (!filename) {
            printf("Error: Variable '%s' not found in memory.\n", filename_var);
            return;
        }
        printf("Reading file %s\n", filename);

        FILE *file = fopen(filename, "r");
        if (!file) {
            printf("Error: Could not open file '%s'\n", filename);
            return;
        }
        printf("File opened successfully\n");

        // Read file content
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        rewind(file);
        fread(value_buffer, 1, size, file);
        value_buffer[size] = '\0';
        fclose(file);
        printf("File content read successfully\n");
    }
    else {
        // Case 3: Direct value (e.g., number, string, another var)
        const char *mem_value = mem_read(proc->mem_low, proc->mem_high, inst->arg2);
        if (mem_value) {
            strncpy(value_buffer, mem_value, sizeof(value_buffer) - 1);
        } else {
            strncpy(value_buffer, inst->arg2, sizeof(value_buffer) - 1);
        }
        value_buffer[sizeof(value_buffer) - 1] = '\0';
    }
    // Store the final result in memory as variable `x`
    for (int i = proc->mem_low; i <= proc->mem_high; i++) {
        if (memory_pool[i].name[0] == '\0' || strcmp(memory_pool[i].name, inst->arg1) == 0) {
            printf("Assigning %s to %s\n", value_buffer, inst->arg1);
            mem_write(i, inst->arg1, value_buffer);
            return;
        }
    }
    printf("Error: No space to assign variable '%s'\n", inst->arg1);
}

void exec_write_file(pcb_t *proc, instruction_t *inst) {
    FILE *file = fopen(inst->arg1, "w");
    if (!file) {
        printf("Error: could not open file %s\n", inst->arg1);
        return;
    }
    fprintf(file, "%s", inst->arg2);
    printf("Writing %s to %s\n", inst->arg2, inst->arg1);
    fclose(file);
}

void exec_read_file(pcb_t *proc, instruction_t *inst) {
    FILE *file = fopen(inst->arg1, "r");
    if (!file) {
        printf("Error: could not open file %s\n", inst->arg1);
        return;
    }
    printf("File opened successfully\n");
    char buffer[100];
    fgets(buffer, sizeof(buffer), file);
    printf("Read %s from %s\n", buffer, inst->arg1);
    fclose(file);
}
void exec_print_from_to(pcb_t *proc, instruction_t *inst){
    const char *val1 = mem_read(proc->mem_low, proc->mem_high, inst->arg1);
    const char *val2 = mem_read(proc->mem_low, proc->mem_high, inst->arg2);
    // If val1 or val2 aren't found, assume direct numbers
    int from = val1 ? atoi(val1) : atoi(inst->arg1);
    int to   = val2 ? atoi(val2) : atoi(inst->arg2);
    printf("Printing from %d to %d\n", from, to);
    if (from > to) {
        for (int i = from; i >= to; i--) {
            printf("%d ", i);
        }
    }
    else {
        for (int i = from; i <= to; i++) {
            printf("%d ", i);
        }
    }
    printf("\n");

}
// void exec_semWait(pcb_t *proc, instruction_t *inst, Scheduler* scheduler){
//     char* name = inst->arg1;
//     sem_wait(name,proc,scheduler);
// }
// void exec_semSignal(instruction_t *inst, Scheduler* scheduler){
//     char* name = inst->arg1;
//     sem_signal(name,scheduler);
// }