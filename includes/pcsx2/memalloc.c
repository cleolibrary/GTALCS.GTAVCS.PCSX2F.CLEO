/*
 * pmpa.c
 * Part of pmpa
 * Copyright (c) 2014 Philip Wernersbach
 *
 * Dual-Licensed under the Public Domain and the Unlicense.
 * Choose the one that you prefer.
 */

#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include "memalloc.h"

typedef struct {
	pmpa_memory_int size;
	bool allocated;
	char data;
} pmpa_memory_block;

#define PMPA_MEMORY_BLOCK_HEADER_SIZE ( offsetof(pmpa_memory_block, data) )
#define PMPA_FIRST_VALID_ADDRESS_IN_POOL master_memory_block
#define PMPA_LAST_VALID_ADDRESS_IN_POOL (PMPA_FIRST_VALID_ADDRESS_IN_POOL + master_memory_block_size)
#define PMPA_POINTER_IS_IN_RANGE(a, b, c) ( ((a) < ((b) + (c))) && ((a) >=  (b)) )
#define PMPA_POINTER_IS_IN_POOL(a) PMPA_POINTER_IS_IN_RANGE(a, PMPA_FIRST_VALID_ADDRESS_IN_POOL, master_memory_block_size)

#ifndef MEM_CUSTOM_TOTAL_SIZE
#define MEM_CUSTOM_TOTAL_SIZE 100000
#endif

static pmpa_memory_block master_memory_block[MEM_CUSTOM_TOTAL_SIZE] = { { MEM_CUSTOM_TOTAL_SIZE - PMPA_MEMORY_BLOCK_HEADER_SIZE, false, 0 } };
static pmpa_memory_int   master_memory_block_size = MEM_CUSTOM_TOTAL_SIZE;

/*
 * Internal functions.
 */

static void concat_sequential_blocks(pmpa_memory_block *memory_block, bool is_allocated)
{
	pmpa_memory_block *current_memory_block = memory_block;
	pmpa_memory_block *next_memory_block = NULL;
	
	if (current_memory_block->allocated != is_allocated)
		return;
	
	while ( (next_memory_block = current_memory_block + current_memory_block->size + PMPA_MEMORY_BLOCK_HEADER_SIZE) && 
			PMPA_POINTER_IS_IN_POOL(next_memory_block + PMPA_MEMORY_BLOCK_HEADER_SIZE) && 
			(next_memory_block->allocated == is_allocated) )
				current_memory_block->size += next_memory_block->size + PMPA_MEMORY_BLOCK_HEADER_SIZE;
	
	
}

static pmpa_memory_block *find_first_block(bool is_allocated, pmpa_memory_int min_size)
{
	pmpa_memory_block *memory_block = PMPA_FIRST_VALID_ADDRESS_IN_POOL;
	
	while (PMPA_POINTER_IS_IN_POOL(memory_block + sizeof(pmpa_memory_block))) {
		/* If we're trying to find an block, then defragment the pool as we go along.
		 * This incurs a minor speed penalty, but not having to spend time
		 * iterating over a fragmented pool makes up for it. */
		if (is_allocated == false)
			concat_sequential_blocks(memory_block, is_allocated);
		
		if ( (memory_block->allocated == is_allocated) && (memory_block->size >= min_size) ) {
			return memory_block;
		} else {
			memory_block += memory_block->size + PMPA_MEMORY_BLOCK_HEADER_SIZE;
		}
	}
	
	return NULL;
}

static void split_block(pmpa_memory_block *memory_block, pmpa_memory_int size)
{
	pmpa_memory_block *second_memory_block = memory_block + size + PMPA_MEMORY_BLOCK_HEADER_SIZE;
	pmpa_memory_block *original_second_memory_block = memory_block + memory_block->size + PMPA_MEMORY_BLOCK_HEADER_SIZE;
	pmpa_memory_int original_memory_block_size = memory_block->size;
	
	memory_block->allocated = false;
	
	/* We can't split this block if there's not enough room to create another one. */
	if ( PMPA_POINTER_IS_IN_RANGE((second_memory_block + PMPA_MEMORY_BLOCK_HEADER_SIZE), 0, original_second_memory_block) &&
	   ( PMPA_POINTER_IS_IN_POOL(second_memory_block + sizeof(pmpa_memory_block)) ) ) {
		memory_block->size = size;
		
		second_memory_block->size = original_memory_block_size - (size + PMPA_MEMORY_BLOCK_HEADER_SIZE);
		second_memory_block->allocated = false;
	}
}

/*
 * Externally accessible C memory functions.
 */

void *pmpa_malloc(size_t size)
{
	pmpa_memory_block *memory_block = find_first_block(false, size);
	
	if (memory_block) {
		split_block(memory_block, size);
		memory_block->allocated = true;
		
		return &(memory_block->data);
	} else {
		return NULL;
	}
}

void *pmpa_calloc(size_t nelem, size_t elsize)
{
	pmpa_memory_int ptr_size = nelem * elsize;
	void *ptr = pmpa_malloc(ptr_size);
	
	if (ptr) {
		memset(ptr, 0, ptr_size);
		
		return ptr;
	} else {
		return NULL;
	}
}

void *pmpa_realloc(void *ptr, size_t size)
{
	pmpa_memory_block *memory_block = NULL;
	pmpa_memory_block *new_memory_block = NULL;
	
	pmpa_memory_int memory_block_original_size = 0;
	
	/* If ptr is NULL, realloc() behaves like malloc(). */
	if (!ptr)
		return pmpa_malloc(size);
	
	memory_block = (pmpa_memory_block*)((char*)ptr - PMPA_MEMORY_BLOCK_HEADER_SIZE);
	memory_block_original_size = memory_block->size;
	
	/* Try to cheat by concatenating the current block with contiguous 
	 * empty blocks after it, and seeing if the new block is big enough. */
	memory_block->allocated = false;
	concat_sequential_blocks(memory_block, memory_block->allocated);
	memory_block->allocated = true;
	
	if (memory_block->size >= size) {
		/* The new block is big enough, split it and use it. */
		split_block(memory_block, size);
		memory_block->allocated = true;
		
		return &(memory_block->data);
	} else {
		/* The new block is not big enough. */ 
		
		/* Restore the memory block's original size. */
		split_block(memory_block, memory_block_original_size);
		memory_block->allocated = true;
		
		/* Find another block and try to use that. */
		if ( !(new_memory_block = find_first_block(false, size)) )
			return NULL;
			
		split_block(new_memory_block, size);
		new_memory_block->allocated = true;
		
		memcpy(&(new_memory_block->data), &(memory_block->data), memory_block->size);
		
		pmpa_free(&(memory_block->data));
		return &(new_memory_block->data);
	}
	
	return NULL;
}

void pmpa_free(void *ptr)
{
	pmpa_memory_block* memory_block = (pmpa_memory_block*)((char*)ptr - PMPA_MEMORY_BLOCK_HEADER_SIZE);
	
	if (ptr == NULL)
		return;
	
	memory_block->allocated = false;
}

void* AllocMemBlock(size_t noOfBytes)
{
	return pmpa_malloc(noOfBytes);
}

void FreeMemBlock(void* ptr)
{
	return pmpa_free(ptr);
}

#ifdef __cplusplus
void* operator new(size_t size)
{
	void* p = AllocMemBlock(size);
	return p;
}

void operator delete(void* p)
{
	FreeMemBlock(p);
}
#endif

void* __wrap_malloc(size_t size)
{
	return pmpa_malloc(size);
}

void* __wrap_calloc(size_t n, size_t size)
{
	return pmpa_calloc(n, size);
}

void* __wrap_realloc(void* p, size_t size)
{
	return pmpa_realloc(p, size);
}

void __wrap_free(void* ptr)
{
	return pmpa_free(ptr);
}
