#include "../include/os.h"
#include "../include/memory.h"
#include "../include/semaphore.h"
#include "../include/scheduler_interface.h"
#include "../include/gui.h"
// #include "../include/memory.h"
// #include "../include/semaphore.h"
// #include "../include/scheduler_interface.h"

// External declaration of scheduler
extern Scheduler* scheduler;

void exec_print(pcb_t *proc, instruction_t *inst){
    char* print_val = mem_read(proc->mem_low, proc->mem_high, inst->arg1);
    if (print_val) {
        // Create buffer for the log message with process ID prefix
        char log_buffer[512];
        snprintf(log_buffer, sizeof(log_buffer), "Process %d output: %s", proc->pid, print_val);
        log_message(log_buffer);
    } else {
        // Log an error if the variable isn't found
        char log_buffer[256];
        snprintf(log_buffer, sizeof(log_buffer), "Process %d: Error - Variable '%s' not found for printing", 
                 proc->pid, inst->arg1);
        log_message(log_buffer);
    }
}

void exec_assign(pcb_t *proc, instruction_t *inst){
    char value_buffer[100];
    if (strcmp(inst->arg2, "input") == 0) {
        // Case 1: Input from user using GUI
        int input_value = get_program_input(proc->pid);
        
        // Check if text input was provided
        if (is_program_text_input()) {
            // Get the text input and copy it to the value buffer
            const char* text_input = get_program_text_input();
            strncpy(value_buffer, text_input, sizeof(value_buffer) - 1);
            value_buffer[sizeof(value_buffer) - 1] = '\0';
            
            // Log the text input
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Process %d received text input: %s", proc->pid, text_input);
            log_message(log_msg);
        } else {
            // Handle numeric input
            snprintf(value_buffer, sizeof(value_buffer), "%d", input_value);
            
            // Log the numeric input
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Process %d received input: %d", proc->pid, input_value);
            log_message(log_msg);
        }
    }
    else if(strncmp(inst->arg2, "readFile", 8) == 0){
        // Case 2: Nested instruction: readFile a
        char filename_var[256];  // Allocate a buffer to hold the filename
        sscanf(inst->arg2 + 9, "%255s", filename_var);

        char *filename = mem_read(proc->mem_low, proc->mem_high, filename_var);
        if (!filename) {
            char log_msg[256];
            // Limit the variable name length to prevent buffer overflow
            snprintf(log_msg, sizeof(log_msg), "Error: Variable '%.100s' not found in memory.", filename_var);
            log_message(log_msg);
            return;
        }
        
        // Use GUI to read file content
        char* file_content = get_file_content(filename);
        if (file_content[0] != '\0') {
            strncpy(value_buffer, file_content, sizeof(value_buffer) - 1);
            value_buffer[sizeof(value_buffer) - 1] = '\0';
        } else {
            // If file is empty or couldn't be read, set to empty string
            value_buffer[0] = '\0';
        }
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
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Assigning %s to %s", value_buffer, inst->arg1);
            log_message(log_msg);
            mem_write(i, inst->arg1, value_buffer);
            return;
        }
    }
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Error: No space to assign variable '%s'", inst->arg1);
    log_message(log_msg);
}

void exec_write_file(pcb_t *proc, instruction_t *inst) {
    // Wait for file resource
    sem_wait(RESOURCE_FILE, proc, scheduler);
    
    char* file_name = mem_read(proc->mem_low, proc->mem_high, inst->arg1);
    if (!file_name) {
        log_message("Error: File name variable not found in memory");
        sem_signal(RESOURCE_FILE, scheduler);
        return;
    }
    
    char* write_val = mem_read(proc->mem_low, proc->mem_high, inst->arg2);
    if (!write_val) {
        log_message("Error: Content variable not found in memory");
        sem_signal(RESOURCE_FILE, scheduler);
        return;
    }
    
    // Use GUI to write file
    int success = write_file_content(file_name, write_val);
    
    if (!success) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Process %d failed to write to file %s", proc->pid, file_name);
        log_message(buffer);
    } else {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Process %d wrote to file %s", proc->pid, file_name);
        log_message(buffer);
    }
    
    // Release file resource
    sem_signal(RESOURCE_FILE, scheduler);
}

void exec_read_file(pcb_t *proc, instruction_t *inst) {
    // Wait for file resource
    sem_wait(RESOURCE_FILE, proc, scheduler);
    
    // Get file content through GUI
    char* content = get_file_content(inst->arg1);
    
    if (content[0] != '\0') { // If content was successfully read
        // Find a free memory location to store the file content
        int stored = 0;
        for (int i = proc->mem_low; i <= proc->mem_high; i++) {
            if (memory_pool[i].name[0] == '\0') {
                // Store file content in a variable named "fileContent"
                mem_write(i, "fileContent", content);
                stored = 1;
                
                char buffer[256];
                snprintf(buffer, sizeof(buffer), "Process %d read file %s into memory", proc->pid, inst->arg1);
                log_message(buffer);
                break;
            }
        }
        
        if (!stored) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "Process %d: No memory available to store file content", proc->pid);
            log_message(buffer);
        }
    } else {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Process %d failed to read file %s", proc->pid, inst->arg1);
        log_message(buffer);
    }
    
    // Release file resource
    sem_signal(RESOURCE_FILE, scheduler);
}
void exec_print_from_to(pcb_t *proc, instruction_t *inst){
    char *val1 = mem_read(proc->mem_low, proc->mem_high, inst->arg1);
    char *val2 = mem_read(proc->mem_low, proc->mem_high, inst->arg2);
    // If val1 or val2 aren't found, assume direct numbers
    int from = val1 ? atoi(val1) : atoi(inst->arg1);
    int to   = val2 ? atoi(val2) : atoi(inst->arg2);
    
    // Log the range
    char range_msg[256];
    snprintf(range_msg, sizeof(range_msg), "Process %d printing from %d to %d:", proc->pid, from, to);
    log_message(range_msg);
    
    // Generate the sequence
    char sequence[1024] = "";
    char temp[16];
    int i;
    
    if (from > to) {
        for (i = from; i >= to; i--) {
            snprintf(temp, sizeof(temp), "%d ", i);
            strncat(sequence, temp, sizeof(sequence) - strlen(sequence) - 1);
        }
    } else {
        for (i = from; i <= to; i++) {
            snprintf(temp, sizeof(temp), "%d ", i);
            strncat(sequence, temp, sizeof(sequence) - strlen(sequence) - 1);
        }
    }
    
    // Log the sequence
    log_message(sequence);
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

    mem_write(idx + 1, "state", state_type_to_string(proc->state));

    snprintf(str, sizeof str, "%d", proc->priority);
    mem_write(idx + 2, "priority", str);

    memset(str, 0, sizeof str); // Clear the buffer
    snprintf(str, sizeof str, "%d", proc->pc);
    mem_write(idx + 3, "pc", str);

    memset(str, 0, sizeof str); // Clear the buffer
    snprintf(str, sizeof str, "%d", proc->time_in_queue);
    mem_write(idx + 7, "time_in_queue", str);
}

// void exec_semWait(pcb_t *proc, instruction_t *inst, Scheduler* scheduler){
//     char* name = inst->arg1;
//     sem_wait(name,proc,scheduler);
// }
// void exec_semSignal(instruction_t *inst, Scheduler* scheduler){
//     char* name = inst->arg1;
//     sem_signal(name,scheduler);
// }