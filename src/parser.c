#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h" 
int parse_program(const char *path, instruction_t **out_code) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    instruction_t *code = malloc(sizeof(instruction_t) * MAX_LINE_LEN);
    int count = 0;
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof line, f)) {
        // trim newline, tokenize by spaces
        char *tok = strtok(line, " \t\n");
        if (!tok) continue;
        instruction_t inst = {0};
        if (strcmp(tok, "assign") == 0) inst.type = INST_ASSIGN;
        else if (strcmp(tok, "print") == 0) inst.type = INST_PRINT;
        else if (strcmp(tok, "writeFile") == 0) inst.type = INST_WRITE_FILE;
        else if (strcmp(tok, "readFile") == 0) inst.type = INST_READ_FILE;
        else if (strcmp(tok, "printFromTo") == 0) inst.type = INST_PRINT_FROM_TO;
        else if (strcmp(tok, "semWait") == 0) inst.type = INST_SEM_WAIT;
        else if (strcmp(tok, "semSignal") == 0) inst.type = INST_SEM_SIGNAL;
        // ... handle all cases ...
        // then read args: arg1 = strtok(NULL), arg2 = strtok(NULL)
        char *a1 = strtok(NULL, " \n");
        char *a2 = strtok(NULL, "\n");
        if (a1) strncpy(inst.arg1, a1, sizeof inst.arg1-1);
        if (a2) strncpy(inst.arg2, a2, sizeof inst.arg2-1);
        code[count++] = inst;
    }
    fclose(f);
    *out_code = code;
    return count;
}
void print_instruction(const instruction_t *inst) {
    const char *type_str;
    switch (inst->type) {
        case INST_ASSIGN:         type_str = "assign"; break;
        case INST_PRINT:          type_str = "print"; break;
        case INST_WRITE_FILE:     type_str = "writeFile"; break;
        case INST_READ_FILE:      type_str = "readFile"; break;
        case INST_PRINT_FROM_TO:  type_str = "printFromTo"; break;
        case INST_SEM_WAIT:       type_str = "semWait"; break;
        case INST_SEM_SIGNAL:     type_str = "semSignal"; break;
        default:                  type_str = "unknown"; break;
    }
    printf("Instruction: %-14s Arg1: %-10s Arg2: %-10s\n",
           type_str, inst->arg1, inst->arg2);
}

// int main() {
//     const char *filename = "../programs/Program_1.txt";  // Path to the program file
//     instruction_t *code = NULL;

//     int n_instructions = parse_program(filename, &code);

//     if (n_instructions < 0) {
//         printf("Failed to open or parse the file: %s\n", filename);
//         return 1;
//     }

//     printf("Parsed %d instruction(s):\n", n_instructions);
//     for (int i = 0; i < n_instructions; i++) {
//         print_instruction(&code[i]);
//     }

//     // Free the dynamically allocated memory
//     free(code);

//     return 0;
// }
