#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os.h"

typedef struct Node {
    pcb_t* pcb;
    struct Node* next;
} Node;
