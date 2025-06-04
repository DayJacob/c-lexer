#pragma once

#include "../tokens.h"
#include "assert.h"
#include "dynarray.h"
#include <stdlib.h>

typedef enum {
  PRGM,
  FUNC_DECL,
  STMT,
  EXPR_BINOP,
  EXPR_UNOP,
  NUM_LIT,
  IDENT_NODE,
  PARAM,
  FUNC_CALL
} NodeType;
typedef enum { OP_PLUS, OP_MINUS, OP_TIMES, OP_DIV } BinOpType;
typedef enum { OP_LOGNEG, NUM_NEG, NUM_POS } UnOpType;
typedef enum { STMT_RET, VAR_DECL } StmtType;

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

    struct {
      dyn_array *func_decls;
    } ast_prgm;

    struct {
      TokenType ret_type;
      char *ident;
      dyn_array *params;
      dyn_array *stmts;
    } ast_func_decl;

    struct {
      StmtType type;
      char *ident;
      struct ast_node *expr;
    } ast_stmt;

    struct {
      TokenType param_type;
      char *ident;
    } ast_param;

    struct {
      char *ident;
      dyn_array *args;
    } ast_func_call;
  };
} ast_node;

ast_node *create_binop(ast_node *left, ast_node *right, BinOpType op);
ast_node *create_unop(ast_node *right, UnOpType);
ast_node *create_num(double num);
ast_node *create_ident(char *ident);
ast_node *create_prgm();
ast_node *create_funcdecl(TokenType ret, char *ident);
ast_node *create_stmt(StmtType type, char *ident, ast_node *expr);
ast_node *create_param(TokenType type, char *ident);
ast_node *create_funccall(char *ident);

void ast_traverse(ast_node *root);
void ast_destroy(ast_node *root);
