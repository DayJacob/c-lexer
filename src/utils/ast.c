#include "ast.h"

ast_node *create_binop(ast_node *left, ast_node *right, BinOpType op) {
  ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  node->type = EXPR_BINOP;
  node->ast_binary_op.type = op;
  node->ast_binary_op.left = left;
  node->ast_binary_op.right = right;
  return node;
}

ast_node *create_num(double num) {
  ast_node *node = (ast_node *)malloc(sizeof(ast_node));
  node->type = NUM_LIT;
  node->num_lit = num;
  return node;
}
