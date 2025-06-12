#pragma once

#include "../tokens.h"
#include "arena.h"
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
  FUNC_CALL,
  IF_STMT,
  ELSE_STMT,
  SCOPE,
  WHILE_STMT
} NodeType;

typedef enum {
  OP_PLUS,
  OP_MINUS,
  OP_TIMES,
  OP_DIV,
  OP_GE,
  OP_GT,
  OP_LE,
  OP_LT,
  OP_EQEQ,
  OP_NEQ
} BinOpType;

typedef enum { OP_LOGNEG, NUM_NEG, NUM_POS } UnOpType;
typedef enum { STMT_RET, VAR_DECL } StmtType;

// TODO: Refactor tagged union?
typedef struct ast_node {
  NodeType type;
  TokenType value;

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
      char *ident;
      dyn_array *params;
      struct ast_node *scope;
    } ast_func_decl;

    // TODO: Consider adding identifiers to scopes
    struct {
      dyn_array *stmts;
    } ast_scope;

    struct {
      StmtType type;
      char *ident;
      struct ast_node *expr;
    } ast_stmt;

    struct {
      char *ident;
    } ast_param;

    struct {
      char *ident;
      dyn_array *args;
    } ast_func_call;

    struct {
      struct ast_node *pred, *scope, *alt;
    } ast_if_stmt;

    struct {
      struct ast_node *scope;
    } ast_else_stmt;

    struct {
      struct ast_node *pred, *scope;
    } ast_while_stmt;
  };
} ast_node;

ast_node *create_binop(ast_node *left, ast_node *right, BinOpType op);
ast_node *create_unop(ast_node *right, UnOpType);
ast_node *create_num(double num, TokenType value);
ast_node *create_ident(char *ident);
ast_node *create_prgm();
ast_node *create_funcdecl(TokenType ret, char *ident, ast_node *scope);
ast_node *create_stmt(StmtType type, TokenType value, char *ident,
                      ast_node *expr);
ast_node *create_param(TokenType type, char *ident);
ast_node *create_funccall(char *ident);
ast_node *create_scope();
ast_node *create_if_stmt(ast_node *pred, ast_node *scope, ast_node *alt);
ast_node *create_else_stmt(ast_node *scope);
ast_node *create_while_stmt(ast_node *pred, ast_node *scope);

void ast_destroy(ast_node *root);
