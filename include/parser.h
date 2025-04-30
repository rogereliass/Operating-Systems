#ifndef PARSER_H
#define PARSER_H

#include "os.h"

// Load program from path; returns number of instructions, or −1 on error
instruction_t* parse_program(char *path) ;

#endif // PARSER_H
