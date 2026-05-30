#include <stdio.h>
#include <stdlib.h>

#include "memory.h"

void *xmalloc(size_t size) {
  void *ptr = malloc(size);
  if (!ptr) {
    perror("Fatal: memory allocation failed");
    exit(1);
  }
  return ptr;
}

void *xrealloc(void *ptr, size_t size) {
  void *new_ptr = realloc(ptr, size);
  if (!new_ptr && size > 0) {
    perror("Fatal: memory reallocation failed");
    exit(1);
  }
  return new_ptr;
}
