#ifndef PARSER_H
#define PARSER_H

#include "os.h"

// Load program from path; returns number of instructions, or −1 on error
int parse_program(const char *path, instruction_t **out_code);

#endif // PARSER_H
