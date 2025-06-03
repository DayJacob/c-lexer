#include "str.h"

char at(str string, size_t i) {
  assert(i < string.len, "Index out of bounds");
  return string.chars[i];
}

char *dupl(str string, size_t start, size_t len) {
  assert(start + len < string.len, "Index out of bounds");
  char *result = (char *)malloc(sizeof(char) * len + 1);
  assert(result != NULL, "Alloc failed");

  size_t i = 0;
  while (i < len) {
    result[i] = string.chars[start + i];
    ++i;
  }

  return result;
}
