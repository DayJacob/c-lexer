#include "ast.h"
#include "dynarray.h"

arena_t alloc;

ast_node *create_binop(ast_node *left, ast_node *right, BinOpType op) {
  // OLD: ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = EXPR_BINOP;
  node->ast_binary_op.type = op;
  node->ast_binary_op.left = left;
  node->ast_binary_op.right = right;
  node->value = (left->value == FLOAT || right->value == FLOAT) ? FLOAT : INT;
  return node;
}

ast_node *create_unop(ast_node *right, UnOpType op) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = EXPR_UNOP;
  node->ast_unary_op.type = op;
  node->ast_unary_op.right = right;
  return node;
}

ast_node *create_num(double num, TokenType value) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = NUM_LIT;
  node->value = (value == INTLIT) ? INT : FLOAT;
  node->num_lit = num;
  return node;
}

ast_node *create_ident(char *ident) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = IDENT_NODE;
  node->value = EMPTY;
  node->ident = ident;
  return node;
}

ast_node *create_prgm() {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = PRGM;
  node->value = EMPTY;
  node->ast_prgm.func_decls = dyn_init(2);
  return node;
}

ast_node *create_funcdecl(TokenType ret, char *ident, ast_node *scope) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = FUNC_DECL;
  node->value = ret;
  node->ast_func_decl.ident = ident;
  node->ast_func_decl.params = dyn_init(2);
  node->ast_func_decl.scope = scope;
  return node;
}

ast_node *create_stmt(StmtType type, TokenType value, char *ident,
                      ast_node *expr) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = STMT;
  node->value = value;
  node->ast_stmt.type = type;
  node->ast_stmt.ident = ident;
  node->ast_stmt.expr = expr;
  return node;
}

ast_node *create_param(TokenType type, char *ident) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = PARAM;
  node->value = type;
  node->ast_param.ident = ident;
  return node;
}

ast_node *create_funccall(char *ident) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = FUNC_CALL;
  node->value = EMPTY;
  node->ast_func_call.ident = ident;
  node->ast_func_call.args = dyn_init(2);
  return node;
}

ast_node *create_scope() {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = SCOPE;
  node->value = EMPTY;
  node->ast_scope.stmts = dyn_init(4);
  return node;
}

ast_node *create_if_stmt(ast_node *pred, ast_node *scope, ast_node *alt) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = IF_STMT;
  node->value = EMPTY;
  node->ast_if_stmt.pred = pred;
  node->ast_if_stmt.scope = scope;
  node->ast_if_stmt.alt = alt;
  return node;
}

ast_node *create_else_stmt(ast_node *scope) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = ELSE_STMT;
  node->value = EMPTY;
  node->ast_else_stmt.scope = scope;
  return node;
}

ast_node *create_while_stmt(ast_node *pred, ast_node *scope) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = WHILE_STMT;
  node->value = EMPTY;
  node->ast_while_stmt.pred = pred;
  node->ast_while_stmt.scope = scope;
  return node;
}

void ast_destroy(ast_node *root) {
  dyn_array *stack = dyn_init(5);
  ast_node *curr = root;

  dyn_push(stack, curr);
  while (stack->len > 0 && curr != NULL) {
    curr = dyn_pop(stack);

    switch (curr->type) {

      case EXPR_BINOP: {
        dyn_push(stack, curr->ast_binary_op.left);
        dyn_push(stack, curr->ast_binary_op.right);
      } break;

      case EXPR_UNOP: {
        dyn_push(stack, curr->ast_unary_op.right);
      } break;

      case FUNC_DECL: {
        for (size_t i = 0; i < curr->ast_func_decl.params->len; ++i)
          dyn_push(stack, dyn_get(curr->ast_func_decl.params, i));

        dyn_destroy(curr->ast_func_decl.params);

        dyn_push(stack, curr->ast_func_decl.scope);

        free(curr->ast_func_decl.ident);
        curr->ast_func_decl.ident = NULL;
      } break;

      case SCOPE: {
        for (size_t i = 0; i < curr->ast_scope.stmts->len; ++i)
          dyn_push(stack, dyn_get(curr->ast_scope.stmts, i));

        dyn_destroy(curr->ast_scope.stmts);
      } break;

      case PRGM: {
        for (size_t i = 0; i < curr->ast_prgm.func_decls->len; ++i)
          dyn_push(stack, dyn_get(curr->ast_prgm.func_decls, i));

        dyn_destroy(curr->ast_prgm.func_decls);
      } break;

      case STMT: {
        if (curr->ast_stmt.expr)
          dyn_push(stack, curr->ast_stmt.expr);

        if (curr->ast_stmt.type == VAR_DECL)
          free(curr->ast_stmt.ident);
      } break;

      case IDENT_NODE: {
        free(curr->ident);

      } break;

      case PARAM: {
        free(curr->ast_param.ident);

      } break;

      case FUNC_CALL: {
        for (size_t i = 0; i < curr->ast_func_call.args->len; ++i)
          dyn_push(stack, dyn_get(curr->ast_func_call.args, i));

        dyn_destroy(curr->ast_func_call.args);
        free(curr->ast_func_call.ident);
      } break;

      case IF_STMT: {
        dyn_push(stack, curr->ast_if_stmt.pred);
        dyn_push(stack, curr->ast_if_stmt.scope);

        if (curr->ast_if_stmt.alt)
          dyn_push(stack, curr->ast_if_stmt.alt);

      } break;

      case WHILE_STMT: {
        dyn_push(stack, curr->ast_if_stmt.pred);
        dyn_push(stack, curr->ast_if_stmt.scope);

      } break;

      default: break;
    }
  }

  dyn_destroy(stack);
}
