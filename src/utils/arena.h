#pragma once

#include <stdlib.h>

typedef struct {
  void *memory;
  size_t cap;
  size_t used;
} arena_t;

extern arena_t alloc;

void arena_init(arena_t *arena, size_t size);
void *arena_alloc(arena_t *arena, size_t size);
void arena_reset(arena_t *arena);
void arena_destroy(arena_t *arena);

#define arena_alloc_type(arena, type) ((type *)arena_alloc(arena, sizeof(type)))
#define arena_alloc_array(arena, type, count)                                  \
  ((type *)arena_alloc(arena, sizeof(type) * (count)))
