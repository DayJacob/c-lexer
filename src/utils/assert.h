#pragma once

#include <stdio.h>

#define assert(_e, _m)                                                         \
  {                                                                            \
    if (!(_e)) {                                                               \
      fprintf(stderr, "%s\n", _m);                                             \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  }
