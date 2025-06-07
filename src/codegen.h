#pragma once

#include "utils/ast.h"
#include <fcntl.h>
#include <stdio.h>

typedef struct {
  char *ident;
  size_t loc;
} symbol;

void generate_x64(ast_node *root, FILE *out);
void generate_arm(ast_node *root, FILE *out);
void generate_llvm(ast_node *root, dyn_array *table, FILE *out);
