#include "../src/utils/ast.h"
#include <stdio.h>

#define assert(_e, _m)                                                         \
  {                                                                            \
    if (!(_e)) {                                                               \
      fprintf(stderr, "%s\n", _m);                                             \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  }

double eval_tree(ast_node *root) {
  if (!root)
    return 0.0;

  switch (root->type) {
  case NUM_LIT:
    return root->num_lit;
  case EXPR_BINOP: {
    double left = eval_tree(root->ast_binary_op.left);
    double right = eval_tree(root->ast_binary_op.right);

    switch (root->ast_binary_op.type) {
    case OP_PLUS:
      return left + right;
    case OP_MINUS:
      return left - right;
    case OP_TIMES:
      return left * right;
    case OP_DIV:
      return (right != 0) ? left / right : 0;
    default:
      return 0;
    }
  }
  case EXPR_UNOP: {
    double right = eval_tree(root->ast_unary_op.right);

    switch (root->ast_unary_op.type) {
    case NUM_NEG:
      return -right;
    case NUM_POS:
    default:
      return right;
    }
  }
  default:
    return 0.0;
  }
}

int main(void) {
  ast_node *root =
      create_binop(create_binop(create_num(4), create_num(3), OP_PLUS),
                   create_num(2), OP_TIMES);

  assert(eval_tree(root) == 14, "Incorrect calculation result");

  ast_destroy(root);

  printf("ALL TESTS PASSED.\n");
  return 0;
}
