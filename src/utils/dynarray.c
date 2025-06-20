#include "dynarray.h"

#include <math.h>
#include <stdio.h>

void dyn_resize(dyn_array *list) {
  list->cap = (size_t)ceil(list->len * 3.0 / 2.0);
  list->el = realloc(list->el, sizeof(*list->el) * list->cap);

  // printf("RESIZE\n");
}

dyn_array *dyn_init(size_t c) {
  dyn_array *list = (dyn_array *)malloc(sizeof(dyn_array));
  if (!list) {
    fprintf(stderr, "Could not allocate memory for dyn_array");
    exit(1);
  }
  list->cap = c;
  list->el = (void **)malloc(sizeof(*list->el) * list->cap);
  if (!list->el) {
    fprintf(stderr, "Could not allocate memory for elements");
    exit(1);
  }

  list->len = 0;

  return list;
}

void dyn_push(dyn_array *list, void *item) {
  if (list->len == list->cap)
    dyn_resize(list);

  list->el[list->len] = item;
  list->len++;
}

void *dyn_pop(dyn_array *list) {
  void *temp = list->el[list->len - 1];

  // list->el = realloc(list->el, sizeof(*list->el) * (list->len - 1));
  list->len--;

  return temp;
}

void *dyn_get(dyn_array *list, size_t idx) {
  assert(idx < list->len, "Index out of bounds");

  return list->el[idx];
}

void dyn_destroy(dyn_array *list) {
  free(list->el);
  list->el = NULL;

  free(list);
  list = NULL;
}
