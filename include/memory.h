#ifndef MEMORY_H
#define MEMORY_H

//#include "os.h"

// Each “word” maps a name → value string
#define MAX_MEM_WORDS 60
typedef struct {
    char name[32];
    char value[64];
} mem_word_t;

// The memory pool
extern mem_word_t memory_pool[MAX_MEM_WORDS];

// Initialize memory (set all entries empty)
void mem_init(void);

// Allocate `n_words` consecutive words; return start index or −1 if fail
int  mem_alloc(int n_words);

// Free a block (by marking words empty)
void mem_free(int start, int n_words);

// Utility: write a name/value into word `idx`
void mem_write(int idx, const char *name, const char *value);

// Utility: read value by name within a range
const char *mem_read(int low, int high, const char *name);

#endif // MEMORY_H
