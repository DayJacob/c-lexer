#include "ast.h"
#include "dynarray.h"

ast_node *create_binop(ast_node *left, ast_node *right, BinOpType op) {
  ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  node->type = EXPR_BINOP;
  node->ast_binary_op.type = op;
  node->ast_binary_op.left = left;
  node->ast_binary_op.right = right;
  return node;
}

ast_node *create_unop(ast_node *right, UnOpType op) {
  ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  node->type = EXPR_UNOP;
  node->ast_unary_op.type = op;
  node->ast_unary_op.right = right;
  return node;
}

ast_node *create_num(double num) {
  ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  node->type = NUM_LIT;
  node->num_lit = num;
  return node;
}

ast_node *create_ident(char *ident) {
  ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  node->type = IDENT_NODE;
  node->ident = ident;
  return node;
}

ast_node *create_prgm() {
  ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  node->type = PRGM;
  node->ast_prgm.func_decls = dyn_init(2);
  return node;
}

ast_node *create_funcdecl(TokenType ret, char *ident) {
  ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  node->type = FUNC_DECL;
  node->ast_func_decl.ret_type = ret;
  node->ast_func_decl.ident = ident;
  node->ast_func_decl.params = dyn_init(2);
  node->ast_func_decl.stmts = dyn_init(4);
  return node;
}

ast_node *create_stmt(StmtType type, char *ident, ast_node *expr) {
  ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  node->type = STMT;
  node->ast_stmt.type = type;
  node->ast_stmt.ident = ident;
  node->ast_stmt.expr = expr;
  return node;
}

ast_node *create_param(TokenType type, char *ident) {
  ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  node->type = PARAM;
  node->ast_param.param_type = type;
  node->ast_param.ident = ident;
  return node;
}

ast_node *create_funccall(char *ident) {
  ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  node->type = FUNC_CALL;
  node->ast_func_call.ident = ident;
  node->ast_func_call.args = dyn_init(2);
  return node;
}

void ast_traverse(ast_node *root) {
  if (!root)
    return;

  if (root->type == PRGM) {
    printf("PRGM:\n");

    for (size_t i = 0; i < root->ast_prgm.func_decls->len; ++i)
      ast_traverse((ast_node *)dyn_get(root->ast_prgm.func_decls, i));

  } else if (root->type == FUNC_DECL) {
    printf("\tFUNC:\n");

    for (size_t i = 0; i < root->ast_func_decl.stmts->len; ++i)
      ast_traverse((ast_node *)dyn_get(root->ast_func_decl.stmts, i));

  } else if (root->type == STMT) {
    printf("\t\tSTMT:");
    switch (root->ast_stmt.type) {
    case STMT_RET:
      printf(" RETURN");
      break;
    case VAR_DECL:
      printf(" VAR DECL");
      break;
    default:
      break;
    }
    printf("\n");

    if (root->ast_stmt.expr)
      ast_traverse(root->ast_stmt.expr);

  } else if (root->type == NUM_LIT)
    printf("\t\t\tliteral: %.1f\n", root->num_lit);

  else if (root->type == EXPR_BINOP) {
    printf("\t\t\tBINOP:");
    switch (root->ast_binary_op.type) {
    case OP_PLUS:
      printf(" +");
      break;
    case OP_MINUS:
      printf(" -");
      break;
    case OP_TIMES:
      printf(" *");
      break;
    case OP_DIV:
      printf(" /");
      break;
    default:
      break;
    }
    printf("\n");

    ast_traverse(root->ast_binary_op.left);
    ast_traverse(root->ast_binary_op.right);

  } else if (root->type == EXPR_UNOP) {
    printf("\t\t\tUNOP:");
    switch (root->ast_unary_op.type) {
    case NUM_NEG:
      printf(" -");
      break;
    case NUM_POS:
      printf(" +");
      break;
    default:
      break;
    }
    printf("\n");

    ast_traverse(root->ast_unary_op.right);
  } else if (root->type == IDENT_NODE)
    printf("\t\t\tIDENT: %s\n", root->ident);
}

void ast_destroy(ast_node *root) {
  dyn_array *stack = dyn_init(5);
  dyn_array *order = dyn_init(5);
  ast_node *curr = root;

  dyn_push(stack, curr);
  while (stack->len > 0 && curr != NULL) {
    curr = dyn_pop(stack);
    dyn_push(order, curr);

    if (curr->type == EXPR_BINOP) {
      dyn_push(stack, curr->ast_binary_op.left);
      dyn_push(stack, curr->ast_binary_op.right);
    } else if (curr->type == EXPR_UNOP) {
      dyn_push(stack, curr->ast_unary_op.right);
    } else if (curr->type == FUNC_DECL) {
      for (size_t i = 0; i < curr->ast_func_decl.params->len; ++i)
        dyn_push(stack, dyn_get(curr->ast_func_decl.params, i));

      dyn_destroy(curr->ast_func_decl.params);

      for (size_t i = 0; i < curr->ast_func_decl.stmts->len; ++i)
        dyn_push(stack, dyn_get(curr->ast_func_decl.stmts, i));

      dyn_destroy(curr->ast_func_decl.stmts);
      free(curr->ast_func_decl.ident);
    } else if (curr->type == PRGM) {
      for (size_t i = 0; i < curr->ast_prgm.func_decls->len; ++i)
        dyn_push(stack, dyn_get(curr->ast_prgm.func_decls, i));

      dyn_destroy(curr->ast_prgm.func_decls);
    } else if (curr->type == STMT) {
      if (curr->ast_stmt.expr)
        dyn_push(stack, curr->ast_stmt.expr);

      if (curr->ast_stmt.type == VAR_DECL)
        free(curr->ast_stmt.ident);
    } else if (curr->type == IDENT_NODE)
      free(curr->ident);

    else if (curr->type == PARAM)
      free(curr->ast_param.ident);

    else if (curr->type == FUNC_CALL) {
      for (size_t i = 0; i < curr->ast_func_call.args->len; ++i)
        dyn_push(stack, dyn_get(curr->ast_func_call.args, i));

      dyn_destroy(curr->ast_func_call.args);
      free(curr->ast_func_call.ident);
    }
  }

  while (order->len > 0) {
    ast_node *node = (ast_node *)dyn_pop(order);
    free(node);
    node = NULL;
  }

  dyn_destroy(stack);
  dyn_destroy(order);
}
