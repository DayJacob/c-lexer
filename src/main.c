#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tokens.h"
#include "utils/dynarray.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Fmt: ./main <file>\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  if (!fp) {
    fprintf(stderr, "Could not open file\n");
    return EXIT_FAILURE;
  }

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *buf = (char *)malloc(fsize + 1);
  if (!buf) {
    fprintf(stderr, "Memory alloc failed\n");
    fclose(fp);
    return EXIT_FAILURE;
  }

  if (fread((void *)buf, fsize, 1, fp) == fsize) {
    fprintf(stderr, "Could not read file into buffer\n");
    fclose(fp);
    free(buf);
    return EXIT_FAILURE;
  }

  dyn_array *toks = dyn_init(fsize / 10);

  tokenize(buf, toks, fsize);
  freeTokens(toks);

  dyn_destroy(toks);

  fclose(fp);
  free(buf);

  return EXIT_SUCCESS;
}
