#include <string.h>
#include "memory.h"
#include <os.h>

mem_word_t memory_pool[MAX_MEM_WORDS];

void mem_init(void) {
    for (int i = 0; i < MAX_MEM_WORDS; i++)
        memory_pool[i].name[0] = '\0';
}

int mem_alloc(int n_words) {
    for (int i = 0; i <= MAX_MEM_WORDS - n_words; i++) {
        int free_block = 1;
        for (int j = 0; j < n_words; j++)
            if (memory_pool[i+j].name[0] != '\0')
                free_block = 0;
        if (free_block) return i;
    }
    return -1;
}

void mem_free(int start, int n_words) {
    for (int i = start; i < start + n_words; i++)
        memory_pool[i].name[0] = '\0';
}

void mem_write(int idx, const char *name, const char *value) {
    strncpy(memory_pool[idx].name, name, sizeof memory_pool[idx].name -1);
    strncpy(memory_pool[idx].value, value, sizeof memory_pool[idx].value -1);
}

const char *mem_read(int low, int high, const char *name) {
    for (int i = low; i <= high; i++) {
        if (strcmp(memory_pool[i].name, name) == 0)
            return memory_pool[i].value;
    }
    return NULL;
}
