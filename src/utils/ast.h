#pragma once

#include "../tokens.h"
#include "arena.h"
#include "assert.h"
#include "dynarray.h"
#include "llvm.h"
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

typedef enum {
  OP_LOGNEG,
  NUM_NEG,
  NUM_POS,
  FLOAT_TOINT,
  INT_TOFLOAT,
  EXTEND,
  TRUNC
} UnOpType;

typedef enum {
  RET_STMT,
  VAR_DECL,
  VAR_ASSIGN,
  REASSIGN,
  IF_STMT,
  ELSE_STMT,
  WHILE_STMT,
  SCOPE
} StmtType;

// TODO: Refactor tagged union?
typedef struct ast_node {
  NodeType type;
  TokenType value;

  union {
    double num_lit; // Number literal

    char *ident; // Identifier/Function parameter

    struct { // Binary operation
      BinOpType type;
      struct ast_node *left, *right;
    } ast_binary_op;

    struct { // Unary operation
      UnOpType type;
      struct ast_node *right;
    } ast_unary_op;

    struct { // Program node
      dyn_array *func_decls;
    } ast_prgm;

    struct { // Function declaration
      char *ident;
      dyn_array *params;
      struct ast_node *scope;
    } ast_func_decl;

    struct { // Function call
      char *ident;
      dyn_array *args;
    } ast_func_call;

    struct { // Statement
      StmtType type;

      union {
        char *ident_decl; // Variable declaration (no assignment)

        struct { // Variable declaration with assignment
          char *ident;
          struct ast_node *expr;
        } var_assign;

        struct { // Return
          struct ast_node *expr;
        } ret;

        struct { // If branch
          struct ast_node *pred, *scope, *alt;
        } if_stmt;

        struct { // Else branch
          struct ast_node *scope;
        } else_stmt;

        struct { // While loop
          struct ast_node *pred, *scope;
        } while_stmt;

        // TODO: Consider adding identifiers to scopes
        struct { // Compound statement
          dyn_array *stmts;
        } scope;
      };
    } ast_stmt;
  };
} ast_node;

ast_node *create_binop(ast_node *left, ast_node *right, BinOpType op);
ast_node *create_unop(ast_node *right, UnOpType op);
ast_node *create_num(double num, TokenType value);
ast_node *create_ident(char *ident, TokenType value);
ast_node *create_prgm();
ast_node *create_funcdecl(TokenType ret, char *ident, ast_node *scope);
ast_node *create_funccall(char *ident, TokenType value);
ast_node *create_param(TokenType value, char *ident);
ast_node *create_vardecl(TokenType value, char *ident);
ast_node *create_varassign(TokenType value, char *ident, ast_node *expr);
ast_node *create_reassign(TokenType value, char *ident, ast_node *expr);
ast_node *create_scope();
ast_node *create_if_stmt(ast_node *pred, ast_node *scope, ast_node *alt);
ast_node *create_else_stmt(ast_node *scope);
ast_node *create_while_stmt(ast_node *pred, ast_node *scope);
ast_node *create_return(TokenType value, ast_node *expr);

UnOpType getImplicitCastOp(TokenType, TokenType);

void ast_destroy(ast_node *root);
