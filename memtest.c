#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef SYSTEM_MALLOC
#define my_free free
#define my_malloc malloc
#define my_realloc realloc
#else
#include "memory_manager.h"
#endif

int rand_between(int min, int max) { return rand() % (max - min + 1) + min; }

#define TEST_SIZE 30

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int main(int argc, char *argv[]) {
    srand(time(NULL));

    char *test_string =
        "Now is the time for all good people to come to the aid "
        "of their country.";

    if (argc > 1) {
        test_string = argv[1];
    }

    char *ptrs[TEST_SIZE];

    // Test malloc, free, and realloc
    for (int ix = 0; ix < TEST_SIZE; ix++) {
        int size;
        if (rand_between(0, 9) == 0) {
            size = rand_between(1024, 1024 * 1024);
        } else {
            size = rand_between(1, strlen(test_string) + 1);
        }
        fprintf(stderr, "\n\n\n[%d] size: %d\n", ix, size);

        ptrs[ix] = my_malloc(size);
        if (ptrs[ix] == NULL) {
            printf("[%d] malloc failed\n", ix);
            exit(1);
        }

        int len_to_copy = MIN(strlen(test_string), size - 1);

        strncpy(ptrs[ix], test_string, len_to_copy);
        ptrs[ix][len_to_copy] = '\0';

        fprintf(stderr, "[%d] '%s'\n", ix, ptrs[ix]);

        int index_to_free = rand_between(0, ix);
        if (ptrs[index_to_free]) {
            fprintf(stderr, "\n[%d] randomly freeing %p ('%s')\n",
                    index_to_free, ptrs[index_to_free], ptrs[index_to_free]);
            my_free(ptrs[index_to_free]);
            fprintf(stderr, "[%d] freed %p\n", index_to_free,
                    ptrs[index_to_free]);
            ptrs[index_to_free] = NULL;
        }

        if (rand_between(0, 4) == 0) {
            int new_size = rand_between(1, 2 * size);
            fprintf(stderr, "[%d] reallocating %p to new size: %d\n", ix,
                    ptrs[ix], new_size);
            ptrs[ix] = my_realloc(ptrs[ix], new_size);
            if (ptrs[ix] == NULL) {
                printf("[%d] realloc failed\n", ix);
                exit(1);
            }
            fprintf(stderr, "[%d] reallocated %p to new size: %d\n", ix,
                    ptrs[ix], new_size);
        }
    }

    for (int ix = 0; ix < TEST_SIZE; ix++) {
        if (ptrs[ix]) {
            fprintf(stderr, "[%d] freeing %p (%s)\n", ix, ptrs[ix], ptrs[ix]);
            my_free(ptrs[ix]);
            fprintf(stderr, "[%d] freed %p\n", ix, ptrs[ix]);
        } else {
            fprintf(stderr, "[%d] already freed\n", ix);
        }
    }
}