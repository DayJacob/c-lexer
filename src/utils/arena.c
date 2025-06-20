#include "arena.h"
#include "assert.h"

void arena_init(arena_t *arena, size_t size) {
  arena->memory = malloc(size);
  assert(arena->memory != NULL, "Allocation failed");

  arena->cap = size;
  arena->used = 0;
}

void *arena_alloc(arena_t *arena, size_t size) {
  assert(arena && arena->memory, "Attempt to allocate to null");

  size_t aligned_used = (arena->used + (8 - 1)) & ~(8 - 1);

  if (aligned_used + size >= arena->cap)
    return NULL;

  void *ptr = arena->memory + aligned_used;
  arena->used = aligned_used + size;
  return ptr;
}

void arena_reset(arena_t *arena) {
  assert(arena, "Attempt to reset null");

  arena->used = 0;
}

void arena_destroy(arena_t *arena) {
  assert(arena && arena->memory, "Attempt to deallocate null");

  free(arena->memory);
  arena->memory = NULL;
  arena->cap = arena->used = 0;
}
