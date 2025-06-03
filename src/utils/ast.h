#pragma once

#include "assert.h"
#include <stdlib.h>

typedef enum { FUNC_DECL, EXPR_BINOP, EXPR_UNOP, NUM_LIT } NodeType;
typedef enum { OP_PLUS, OP_MINUS, OP_TIMES, OP_DIV } BinOpType;
typedef enum { OP_LOGNEG, NUM_NEG, NUM_POS } UnOpType;

typedef struct ast_node {
  NodeType type;
  union {
    double num_lit;

    char *ident;

    struct {
      BinOpType type;
      struct ast_node *left, *right;
    } ast_binary_op;

    struct {
      UnOpType type;
      struct ast_node *right;
    } ast_unary_op;
  };
} ast_node;

ast_node *create_binop(ast_node *left, ast_node *right, BinOpType op);
ast_node *create_num(double num);
