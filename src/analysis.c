#include "analysis.h"
#include "utils/ast.h"

void analyze(ast_node *root) {
  switch (root->type) {
    case PRGM: {
      for (size_t i = 0; i < root->ast_prgm.func_decls->len; ++i)
        analyze((ast_node *)root->ast_prgm.func_decls->el[i]);
    } break;

    case FUNC_DECL: {
      ast_node *scope = root->ast_func_decl.scope;

      for (size_t i = 0; i < scope->ast_stmt.scope.stmts->len; ++i) {
        ast_node *stmt = scope->ast_stmt.scope.stmts->el[i];

        switch (stmt->ast_stmt.type) {
          case RET_STMT: stmt->value = root->value; break;
          default:       break;
        }

        analyze(stmt);
      }
    } break;

    case STMT: {
      ast_node *expr = NULL;
      switch (root->ast_stmt.type) {
        case RET_STMT:   expr = root->ast_stmt.ret.expr; break;
        case VAR_ASSIGN: expr = root->ast_stmt.var_assign.expr; break;
        default:         return;
      }

      analyze(expr);

      // if the types don't match, insert an implicit cast ast_node
      if (root->value != expr->value) {
        ast_node *cast =
            create_unop(expr, getImplicitCastOp(root->value, expr->value));
        cast->value = root->value;

        switch (root->ast_stmt.type) {
          case RET_STMT:   root->ast_stmt.ret.expr = cast; break;
          case VAR_ASSIGN: root->ast_stmt.var_assign.expr = cast; break;
          default:         break; // unreachable
        }
      }
    } break;

    case EXPR_BINOP: {
      analyze(root->ast_binary_op.left);
      analyze(root->ast_binary_op.right);

      if (root->value != root->ast_binary_op.left->value) {
        ast_node *cast = create_unop(
            root->ast_binary_op.left,
            getImplicitCastOp(root->value, root->ast_binary_op.left->value));
        cast->value = root->value;
        root->ast_binary_op.left = cast;
      }

      if (root->value != root->ast_binary_op.right->value) {
        ast_node *cast = create_unop(
            root->ast_binary_op.right,
            getImplicitCastOp(root->value, root->ast_binary_op.right->value));
        cast->value = root->value;
        root->ast_binary_op.right = cast;
      }
    } break;

    case EXPR_UNOP: {
      analyze(root->ast_unary_op.right);

      if (root->value != root->ast_unary_op.right->value) {
        ast_node *cast = create_unop(
            root->ast_unary_op.right,
            getImplicitCastOp(root->value, root->ast_unary_op.right->value));
        cast->value = root->value;
        root->ast_unary_op.right = cast;
      }
    } break;

    default: break;
  }
}
