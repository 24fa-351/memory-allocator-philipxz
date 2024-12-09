#include "memory_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define BLOCK_SIZE (sizeof(BlockMeta))
#define HEAP_SIZE 1024 * 1024  // 1MB initial heap size
#define MAX_HEAP_BLOCKS 1000

typedef struct BlockMeta {
    size_t size;
    struct BlockMeta* next;
} BlockMeta;

typedef struct MinHeap {
    BlockMeta* blocks[MAX_HEAP_BLOCKS];
    size_t size;
} MinHeap;

static MinHeap free_heap;
static void* heap_start = NULL;
static size_t heap_size = 0;

static void heap_insert(MinHeap* heap, BlockMeta* block) {
    if (heap->size >= MAX_HEAP_BLOCKS) {
        fprintf(stderr, "Error: Free heap overflow\n");
        return;
    }
    size_t i = heap->size++;
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (heap->blocks[parent]->size <= block->size) break;
        heap->blocks[i] = heap->blocks[parent];
        i = parent;
    }
    heap->blocks[i] = block;
}

static BlockMeta* heap_extract_min(MinHeap* heap) {
    if (heap->size == 0) return NULL;
    BlockMeta* min_block = heap->blocks[0];
    BlockMeta* last_block = heap->blocks[--heap->size];

    size_t i = 0;
    while (1) {
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;
        size_t smallest = i;

        if (left < heap->size &&
            heap->blocks[left]->size < heap->blocks[smallest]->size)
            smallest = left;
        if (right < heap->size &&
            heap->blocks[right]->size < heap->blocks[smallest]->size)
            smallest = right;

        if (smallest == i) break;

        heap->blocks[i] = heap->blocks[smallest];
        i = smallest;
    }
    heap->blocks[i] = last_block;
    return min_block;
}

void* get_me_blocks(ssize_t how_much) {
    void* ptr = sbrk(0);
    if (sbrk(how_much) == (void*)-1) {
        return NULL;
    }
    return ptr;
}

void* my_malloc(size_t size) {
    printf("malloc(%lu)\n", size);
    size = ALIGN(size + BLOCK_SIZE);

    BlockMeta* block = heap_extract_min(&free_heap);
    // Find the first block that fits
    while (block != NULL) {
        if (block->size >= size) {
            // Split block if large enough
            if (block->size > size + BLOCK_SIZE) {
                BlockMeta* new_block =
                    (BlockMeta*)((char*)block + BLOCK_SIZE + size);
                new_block->size = block->size - size - BLOCK_SIZE;
                heap_insert(&free_heap, new_block);
            }
            block->size = size;
            return (void*)(block + 1);
        }
        block = heap_extract_min(&free_heap);
    }

    // If no block fits, allocate a new one
    if (!heap_start) {
        heap_start = get_me_blocks(HEAP_SIZE);
        if (!heap_start) return NULL;
        heap_size = HEAP_SIZE;
    }
    while (heap_size < size + BLOCK_SIZE) {
        void* new_heap_start = get_me_blocks(HEAP_SIZE);
        if (!new_heap_start) return NULL;
        heap_size += HEAP_SIZE;
    }

    // Allocate a new block
    block = (BlockMeta*)heap_start;
    block->size = size;
    heap_start = (void*)((char*)heap_start + size);
    heap_size -= size + BLOCK_SIZE;
    return (block + 1);
}

void my_free(void* ptr) {
    if (!ptr) {
        return;
    }

    BlockMeta* block = (BlockMeta*)ptr - 1;
    heap_insert(&free_heap, block);
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