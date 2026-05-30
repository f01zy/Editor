#ifndef MEMORY_INCLUDED
#define MEMORY_INCLUDED

#include <stddef.h>

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);

#endif
