/* Preserve CLEO's allocation API while using the injected module's aligned
 * private heap. The old pool returned data at a five-byte header offset,
 * which traps on the EE interpreter when C++ constructors access objects. */
#include <stdlib.h>
#include "../includes/pcsx2/memalloc.h"
#undef malloc
#undef calloc
#undef realloc
#undef free

void* pmpa_malloc(size_t size) { return malloc(size); }
void* pmpa_calloc(size_t count, size_t size) { return calloc(count, size); }
void* pmpa_realloc(void* pointer, size_t size) { return realloc(pointer, size); }
void pmpa_free(void* pointer) { free(pointer); }
void* AllocMemBlock(size_t size) { return malloc(size); }
void FreeMemBlock(void* pointer) { free(pointer); }
