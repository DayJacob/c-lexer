#pragma once

#include "tokens.h"
#include "utils/ast.h"
#include <fcntl.h>
#include <stdio.h>

typedef struct {
  char *ident;
  size_t loc;
  TokenType type;
} symbol;

inline const char *asLLVMType(TokenType);
inline TokenType asBasicType(TokenType);
inline const size_t getAlignment(TokenType);
inline symbol *findInSymTable(dyn_array *table, const char *ident);

void generate_x64(ast_node *root, FILE *out);
void generate_arm(ast_node *root, FILE *out);
void generate_llvm(ast_node *root, dyn_array *table, FILE *out);
