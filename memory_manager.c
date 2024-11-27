#include "memory_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define BLOCK_SIZE (sizeof(BlockMeta))
#define HEAP_SIZE 1024 * 1024  // 1MB initial heap size

typedef struct BlockMeta {
    size_t size;
    struct BlockMeta* next;
} BlockMeta;

static BlockMeta* free_list = NULL;
static void* heap_start = NULL;
static size_t heap_size = 0;

void* get_me_blocks(ssize_t how_much) {
    void* ptr = sbrk(0);
    if (sbrk(how_much) == (void*)-1) {
        return NULL;
    }
    return ptr;
}

void* my_malloc(size_t size) {
    size = ALIGN(size + BLOCK_SIZE);
    BlockMeta* block = free_list;
    BlockMeta** prev = &free_list;

    // Find the first block that fits
    while (block != NULL) {
        if (block->size >= size) {
            *prev = block->next;
            return (block + 1);
        }
        prev = &block->next;
        block = block->next;
    }

    // If no block fits, allocate a new one
    if (!heap_start) {
        heap_start = get_me_blocks(HEAP_SIZE);
        if (!heap_start) {
            return NULL;
        }
        heap_size = HEAP_SIZE;
    }

    while (heap_size < size) {
        void* new_heap_start = get_me_blocks(HEAP_SIZE);
        if (!new_heap_start) {
            return NULL;
        }
        heap_size += HEAP_SIZE;
    }

    block = (BlockMeta*)heap_start;
    block->size = size;
    heap_start = (void*)((char*)heap_start + size);
    heap_size -= size;

    memset(block + 1, 0, size - BLOCK_SIZE);
    return (block + 1);
}

void my_free(void* ptr) {
    if (!ptr) return;
    fprintf(stderr, __FILE__ ": free(%p)\n", ptr);
    BlockMeta* block = (BlockMeta*)ptr - 1;
    block->next = free_list;
    free_list = block;
}

void* my_realloc(void* ptr, size_t size) {
    if (!ptr) {
        fprintf(stderr, __FILE__ "realloc called with NULL pointer\n");
        return my_malloc(size);
    }
    if (size == 0) {
        my_free(ptr);
        return NULL;
    }

    BlockMeta* block = (BlockMeta*)ptr - 1;
    if (block->size >= size) {
        return ptr;
    }

    void* new_ptr = my_malloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size - BLOCK_SIZE);
        my_free(ptr);
    }
    return new_ptr;
}