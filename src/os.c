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
        fflush(stdout); // <-- ADD THIS LINE
        fgets(value_buffer, sizeof value_buffer, stdin);
        value_buffer[strcspn(value_buffer, "\n")] = '\0';  // remove newline
    }
    else if(strncmp(inst->arg2, "readFile", 8) == 0){
        // Case 2: Nested instruction: readFile a
        // char *filename_var = strtok(inst->arg2 + 9, " \n");  // get the next token after "readFile"
        char filename_var[256];  // Allocate a buffer to hold the filename
        sscanf(inst->arg2 + 9, "%255s", filename_var);

        char *filename = mem_read(proc->mem_low, proc->mem_high, filename_var);
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
        // --- START OF CHANGES ---
        if (size < 0) { // Add error checking for ftell
            perror("Error getting file size");
            fclose(file);
            // Maybe set value_buffer to an error string or return?
            // For now, let's make it empty.
            value_buffer[0] = '\0';
            // Skip the rest of the file reading logic
        } else {
            rewind(file);
            // Calculate how many bytes we can actually read safely
            size_t buffer_capacity = sizeof(value_buffer) - 1;
            size_t bytes_to_read = (size < buffer_capacity) ? (size_t)size : buffer_capacity;

            // Read only that many bytes
            size_t bytes_read = fread(value_buffer, 1, bytes_to_read, file);

            // Null-terminate the buffer *after* the bytes actually read
            value_buffer[bytes_read] = '\0';

            // Optional: Check if fread encountered an error
            if (bytes_read != bytes_to_read && ferror(file)) {
                perror("Error reading file content");
            }
             printf("File content read successfully (%zu bytes)\n", bytes_read);
        }
        // --- END OF CHANGES ---
        fclose(file);
    }
    else {
        // Case 3: Direct value (e.g., number, string, another var)
        char *mem_value = mem_read(proc->mem_low, proc->mem_high, inst->arg2);
        if (mem_value) {
            strncpy(value_buffer, mem_value, sizeof(value_buffer) - 1);
        } else {
            strncpy(value_buffer, inst->arg2, sizeof(value_buffer) - 1);
        }
        value_buffer[sizeof(value_buffer) - 1] = '\0';
    }
    // Store the final result in memory as variable `x`
    for (int i = proc->mem_low; i <= proc->mem_low+2; i++) {
        if (memory_pool[i].name[0] == '\0' || strcmp(memory_pool[i].name, inst->arg1) == 0) {
            printf("Assigning %s to %s\n", value_buffer, inst->arg1);
            mem_write(i, inst->arg1, value_buffer);
            return;
        }
    }
    printf("Error: No space to assign variable '%s'\n", inst->arg1);
}

void exec_write_file(pcb_t *proc, instruction_t *inst) {
    char* file_name = mem_read(proc->mem_low, proc->mem_high, inst->arg1);
    FILE *file = fopen(file_name, "w");
    if (!file) {
        printf("Error: could not open file %s\n", inst->arg1);
        return;
    }
    char* write_val = mem_read(proc->mem_low, proc->mem_high, inst->arg2);
    fprintf(file, "%s", write_val);
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
    char *val1 = mem_read(proc->mem_low, proc->mem_high, inst->arg1);
    char *val2 = mem_read(proc->mem_low, proc->mem_high, inst->arg2);
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

char * state_type_to_string(proc_state_t state) {
    switch (state) {
        case NEW: return "NEW";
        case READY: return "READY";
        case RUNNING: return "RUNNING";
        case BLOCKED: return "BLOCKED";
        case TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

// Update the PCB in memory with the new values
// This function assumes that the PCB is stored in memory and that
// the memory pool is already initialized
void update_pcb_in_memory(pcb_t *proc) {
    int idx = proc->pcb_index;
    char str[32];
    // snprintf(str, sizeof str, "%d", proc->pid);
    // mem_write(idx, "pid", str);
    mem_write(idx + 1, "state", state_type_to_string(proc->state));
    // memset(str, 0, sizeof str); // Clear the buffer
    snprintf(str, sizeof str, "%d", proc->priority);
    mem_write(idx + 2, "priority", str);
    memset(str, 0, sizeof str); // Clear the buffer
    snprintf(str, sizeof str, "%d", proc->pc);
    mem_write(idx + 3, "pc", str);
    memset(str, 0, sizeof str); // Clear the buffer
    snprintf(str, sizeof str, "%d", proc->mem_low);
    mem_write(idx + 4, "mem_low", str);
    memset(str, 0, sizeof str); // Clear the buffer
    snprintf(str, sizeof str, "%d", proc->mem_high);
    mem_write(idx + 5, "mem_high", str);
}

// void exec_semWait(pcb_t *proc, instruction_t *inst, Scheduler* scheduler){
//     char* name = inst->arg1;
//     sem_wait(name,proc,scheduler);
// }
// void exec_semSignal(instruction_t *inst, Scheduler* scheduler){
//     char* name = inst->arg1;
//     sem_signal(name,scheduler);
// }