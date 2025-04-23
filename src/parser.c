#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

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
        // ... handle all cases ...
        // then read args: arg1 = strtok(NULL), arg2 = strtok(NULL)
        char *a1 = strtok(NULL, " \n");
        char *a2 = strtok(NULL, " \n");
        if (a1) strncpy(inst.arg1, a1, sizeof inst.arg1-1);
        if (a2) strncpy(inst.arg2, a2, sizeof inst.arg2-1);
        code[count++] = inst;
    }
    fclose(f);
    *out_code = code;
    return count;
}
