#pragma once

#include "assert.h"
#include <stdlib.h>

typedef struct {
  size_t len;
  char *chars;
} str;

char at(str string, size_t i);
char *dupl(str string, size_t start, size_t len);
