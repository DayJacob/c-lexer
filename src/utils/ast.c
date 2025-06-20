#include "ast.h"
#include "dynarray.h"
#include "llvm.h"

arena_t alloc;

ast_node *create_binop(ast_node *left, ast_node *right, BinOpType op) {
  // OLD: ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = EXPR_BINOP;
  node->ast_binary_op.type = op;
  node->ast_binary_op.left = left;
  node->ast_binary_op.right = right;
  node->value = getStrongerType(left->value, right->value);
  return node;
}

ast_node *create_unop(ast_node *right, UnOpType op) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = EXPR_UNOP;
  node->ast_unary_op.type = op;
  node->ast_unary_op.right = right;
  node->value = right->value;
  return node;
}

ast_node *create_num(double num, TokenType value) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = NUM_LIT;
  node->value = (value == INTLIT) ? INT : FLOAT;
  node->num_lit = num;
  return node;
}

ast_node *create_ident(char *ident, TokenType value) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = IDENT_NODE;
  node->value = value;
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

ast_node *create_funccall(char *ident, TokenType value) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = FUNC_CALL;
  node->value = value;
  node->ast_func_call.ident = ident;
  node->ast_func_call.args = dyn_init(2);
  return node;
}

ast_node *create_param(TokenType type, char *ident) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = PARAM;
  node->value = type;
  node->ident = ident;
  return node;
}

ast_node *create_vardecl(TokenType value, char *ident) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = STMT;
  node->value = value;
  node->ast_stmt.type = VAR_DECL;
  node->ast_stmt.ident_decl = ident;
  return node;
}

ast_node *create_varassign(TokenType value, char *ident, ast_node *expr) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = STMT;
  node->value = value;
  node->ast_stmt.type = VAR_ASSIGN;
  node->ast_stmt.var_assign.ident = ident;
  node->ast_stmt.var_assign.expr = expr;
  return node;
}

ast_node *create_reassign(TokenType value, char *ident, ast_node *expr) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = STMT;
  node->value = value;
  node->ast_stmt.type = REASSIGN;
  node->ast_stmt.var_assign.ident = ident;
  node->ast_stmt.var_assign.expr = expr;
  return node;
}

ast_node *create_scope() {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = STMT;
  node->value = EMPTY;
  node->ast_stmt.type = SCOPE;
  node->ast_stmt.scope.stmts = dyn_init(4);
  return node;
}

ast_node *create_if_stmt(ast_node *pred, ast_node *scope, ast_node *alt) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = STMT;
  node->value = EMPTY;
  node->ast_stmt.type = IF_STMT;
  node->ast_stmt.if_stmt.pred = pred;
  node->ast_stmt.if_stmt.scope = scope;
  node->ast_stmt.if_stmt.alt = alt;
  return node;
}

ast_node *create_else_stmt(ast_node *scope) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = STMT;
  node->value = EMPTY;
  node->ast_stmt.type = ELSE_STMT;
  node->ast_stmt.else_stmt.scope = scope;
  return node;
}

ast_node *create_while_stmt(ast_node *pred, ast_node *scope) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = STMT;
  node->value = EMPTY;
  node->ast_stmt.type = WHILE_STMT;
  node->ast_stmt.while_stmt.pred = pred;
  node->ast_stmt.while_stmt.scope = scope;
  return node;
}

ast_node *create_return(TokenType value, ast_node *expr) {
  ast_node *node = arena_alloc_type(&alloc, ast_node);
  node->type = STMT;
  node->value = value;
  node->ast_stmt.type = RET_STMT;
  node->ast_stmt.ret.expr = expr;
  return node;
}

UnOpType getImplicitCastOp(TokenType parent, TokenType child) {
  if (asBasicType(parent) == asBasicType(child))
    return (getAlignment(parent) > getAlignment(child)) ? EXTEND : TRUNC;
  else
    return (asBasicType(parent) == FLOAT) ? INT_TOFLOAT : FLOAT_TOINT;
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

      case PRGM: {
        for (size_t i = 0; i < curr->ast_prgm.func_decls->len; ++i)
          dyn_push(stack, dyn_get(curr->ast_prgm.func_decls, i));

        dyn_destroy(curr->ast_prgm.func_decls);
      } break;

      case STMT: {

        switch (curr->ast_stmt.type) {
          case VAR_DECL: free(curr->ast_stmt.ident_decl); break;
          case VAR_ASSIGN:
            free(curr->ast_stmt.var_assign.ident);
            dyn_push(stack, curr->ast_stmt.var_assign.expr);
            break;
          case RET_STMT: dyn_push(stack, curr->ast_stmt.ret.expr); break;

          case SCOPE:    {
            for (size_t i = 0; i < curr->ast_stmt.scope.stmts->len; ++i)
              dyn_push(stack, dyn_get(curr->ast_stmt.scope.stmts, i));

            dyn_destroy(curr->ast_stmt.scope.stmts);
          } break;

          case IF_STMT: {
            dyn_push(stack, curr->ast_stmt.if_stmt.pred);
            dyn_push(stack, curr->ast_stmt.if_stmt.scope);

            if (curr->ast_stmt.if_stmt.alt)
              dyn_push(stack, curr->ast_stmt.if_stmt.alt);

          } break;

          case WHILE_STMT: {
            dyn_push(stack, curr->ast_stmt.if_stmt.pred);
            dyn_push(stack, curr->ast_stmt.if_stmt.scope);

          } break;

          default: break;
        }

      } break;

      case IDENT_NODE: {
        free(curr->ident);

      } break;

      case PARAM: {
        free(curr->ident);

      } break;

      case FUNC_CALL: {
        for (size_t i = 0; i < curr->ast_func_call.args->len; ++i)
          dyn_push(stack, dyn_get(curr->ast_func_call.args, i));

        dyn_destroy(curr->ast_func_call.args);
        free(curr->ast_func_call.ident);
      } break;

      default: break;
    }
  }

  dyn_destroy(stack);
}
