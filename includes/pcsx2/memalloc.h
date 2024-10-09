/*
 * pmpa.h
 * Part of pmpa
 * Copyright (c) 2014 Philip Wernersbach
 *
 * Dual-Licensed under the Public Domain and the Unlicense.
 * Choose the one that you prefer.
 */

#ifndef HAVE_PMPA_H

#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

typedef uint32_t pmpa_memory_int;

/*
 * Externally accessible C memory functions.
 */

#ifdef __cplusplus
extern "C"
{
#endif
void* pmpa_malloc(size_t size);
void* pmpa_calloc(size_t nelem, size_t elsize);
void* pmpa_realloc(void* ptr, size_t size);
void pmpa_free(void* ptr);
void* AllocMemBlock(size_t noOfBytes);
void FreeMemBlock(void* ptr);
#ifdef __cplusplus
}
#endif

#define malloc(a) pmpa_malloc(a)
#define calloc(a, b) pmpa_calloc(a, b)
#define realloc(a, b) pmpa_realloc(a, b)
#define free(a) pmpa_free(a)

#define HAVE_PMPA_H
#endif