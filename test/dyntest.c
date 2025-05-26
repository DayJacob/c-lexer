#include "../src/libs/dynarray.h"
#include <stdio.h>

#define assert(_e, _m)                                                         \
  {                                                                            \
    if (!(_e)) {                                                               \
      fprintf(stderr, "%s\n", _m);                                             \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  }

int main(void) {
  dyn_array *list = dyn_init(2);
  assert(list->cap == 2, "Incorrect list capacity");
  assert(list->len == 0, "Incorrect list size");

  int num = 4;
  dyn_push(list, &num);
  assert(list->cap == 2, "Incorrect list capacity");
  assert(list->len == 1, "Incorrect list size");

  int *result = (int *)dyn_get(list, 0);
  assert(*result == 4, "Incorrect value push");

  int num2 = 7;
  dyn_push(list, &num2);
  assert(list->cap == 2, "Incorrect list capacity");
  assert(list->len == 2, "Incorrect list size");

  int *result2 = (int *)dyn_get(list, 1);
  assert(*result2 == 7, "Incorrect value push");

  int *pop = (int *)dyn_pop(list);
  assert(list->cap == 2, "Incorrect list capacity");
  assert(list->len == 1, "Incorrect list size");
  assert(*pop == 7, "Incorrect value pop");

  dyn_push(list, &num2);

  int num3 = 2;
  dyn_push(list, &num3);
  assert(list->cap == 3, "Incorrect list resize");
  assert(list->len == 3, "Incorrect list size");

  int myNums[] = {12, 9, 4, 8};
  for (size_t i = 0; i < 4; ++i)
    dyn_push(list, &myNums[i]);

  assert(list->cap == 8, "Incorrect list resize");
  assert(list->len == 7, "Incorrect list size");

  for (size_t i = 0; i < 7; ++i)
    printf("%i ", *((int *)dyn_get(list, i)));
  printf("\n");

  printf("ALL TESTS PASSED.\n");
  dyn_destroy(list);
  return 0;
}
