#pragma once

#include <stdlib.h>

#define assert(_e, _m)                                                         \
  {                                                                            \
    if (!(_e)) {                                                               \
      fprintf(stderr, "%s\n", _m);                                             \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  }

typedef struct {
  size_t len;
  size_t cap;
  void **el;
} dyn_array;

dyn_array *dyn_init(size_t c);
void dyn_push(dyn_array *list, void *item);
void *dyn_pop(dyn_array *list);
void *dyn_get(dyn_array *list, size_t idx);
void dyn_destroy(dyn_array *list);
