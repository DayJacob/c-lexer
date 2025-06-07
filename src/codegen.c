#include "codegen.h"
#include "utils/ast.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static size_t ssa = 1;
static TokenType ret_type;

const char *asLLVMType(TokenType type) {
  switch (type) {

    case INT:  return "i32";
    case VOID: return "void";
    default:   return "";
  }
}

bool isComptimeExpr(ast_node *root) {
  if (!root)
    return true;

  switch (root->type) {
    case NUM_LIT: return true;
    case EXPR_BINOP:
      return isComptimeExpr(root->ast_binary_op.left) &&
             isComptimeExpr(root->ast_binary_op.right);
    case EXPR_UNOP: return isComptimeExpr(root->ast_unary_op.right);

    default: return false;
  }
}

double eval_tree(ast_node *root) {
  if (!root)
    return 0.0;

  switch (root->type) {
    case NUM_LIT:    return root->num_lit;
    case EXPR_BINOP: {
      double left = eval_tree(root->ast_binary_op.left);
      double right = eval_tree(root->ast_binary_op.right);

      switch (root->ast_binary_op.type) {
        case OP_PLUS:  return left + right;
        case OP_MINUS: return left - right;
        case OP_TIMES: return left * right;
        case OP_DIV:   return (right != 0) ? left / right : 0;
        default:       return 0;
      }
    }
    case EXPR_UNOP: {
      double right = eval_tree(root->ast_unary_op.right);

      switch (root->ast_unary_op.type) {
        case NUM_NEG: return -right;
        case NUM_POS:
        default:      return right;
      }
    }
    default: return 0.0;
  }
}

void generate_x64(ast_node *root, FILE *out) {
}

void generate_llvm(ast_node *root, dyn_array *table, FILE *out) {
  if (!root)
    return;

  switch (root->type) {

    case PRGM: {
      for (size_t i = 0; i < root->ast_prgm.func_decls->len; ++i)
        generate_llvm((ast_node *)dyn_get(root->ast_prgm.func_decls, i), table,
                      out);

    } break;

    case FUNC_DECL: {
      ret_type = root->ast_func_decl.ret_type;
      fprintf(out, "define %s @%s() {\n", asLLVMType(ret_type),
              root->ast_func_decl.ident);

      fprintf(out, "\t%%%lu = alloca i32, align 4\n", ssa);
      fprintf(out, "\tstore i32 0, i32* %%%lu, align 4\n", ssa++);

      generate_llvm(root->ast_func_decl.scope, table, out);

      fprintf(out, "}\n");

    } break;

    case SCOPE: {
      for (size_t i = 0; i < root->ast_scope.stmts->len; ++i)
        generate_llvm((ast_node *)dyn_get(root->ast_scope.stmts, i), table,
                      out);

    } break;

    case STMT: {
      switch (root->ast_stmt.type) {

        case VAR_DECL: {
          symbol *var = (symbol *)malloc(sizeof(symbol));
          var->ident = root->ast_stmt.ident;
          var->loc = ssa;
          dyn_push(table, var);

          fprintf(out, "\t%%%lu = alloca i32, align 4\n", ssa++);

          if (isComptimeExpr(root->ast_stmt.expr)) {
            double num = eval_tree(root->ast_stmt.expr);
            fprintf(out, "\tstore i32 %i, i32* %%%lu, align 4\n", (int)num,
                    ssa - 1);

            break;
          }

          generate_llvm(root->ast_stmt.expr, table, out);

        } break;

        case STMT_RET: {
          if (isComptimeExpr(root->ast_stmt.expr)) {
            double num = eval_tree(root->ast_stmt.expr);
            fprintf(out, "\tret i32 %i\n", (int)num);
            break;
          }

          generate_llvm(root->ast_stmt.expr, table, out);

          fprintf(out, "\tret %s %%%lu\n", asLLVMType(ret_type), ssa - 1);

        } break;

        default: break;
      }

    } break;

    case EXPR_BINOP: {
      size_t lhs, rhs;
      bool lhs_comptime = false, rhs_comptime = false;

      if ((lhs_comptime = isComptimeExpr(root->ast_binary_op.left)))
        lhs = (int)eval_tree(root->ast_binary_op.left);
      else
        generate_llvm(root->ast_binary_op.left, table, out);

      if ((rhs_comptime = isComptimeExpr(root->ast_binary_op.right)))
        rhs = (int)eval_tree(root->ast_binary_op.right);
      else
        generate_llvm(root->ast_binary_op.right, table, out);

      bool has_comptime_expr = lhs_comptime || rhs_comptime;

      switch (root->ast_binary_op.type) {

        case OP_PLUS: {
          fprintf(out, "\t%%%lu = add nsw i32 ", ssa);

          if (!lhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%lu, ",
                  (lhs_comptime ? lhs : ssa - (2 - (int)has_comptime_expr)));

          if (!rhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%lu, align 4\n", (rhs_comptime ? rhs : ssa - 1));

          ++ssa;

        } break;

        case OP_TIMES: {
          fprintf(out, "\t%%%lu = mul nsw i32 ", ssa);

          if (!lhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%lu, ",
                  (lhs_comptime ? lhs : ssa - (2 - (int)has_comptime_expr)));

          if (!rhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%lu, align 4\n", (rhs_comptime ? rhs : ssa - 1));

          ++ssa;

        } break;

        case OP_MINUS: {
          fprintf(out, "\t%%%lu = sub nsw i32 ", ssa);

          if (!lhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%lu, ",
                  (lhs_comptime ? lhs : ssa - (2 - (int)has_comptime_expr)));

          if (!rhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%lu, align 4\n", (rhs_comptime ? rhs : ssa - 1));

          ++ssa;

        } break;

        default: break;
      }

    } break;

    case IDENT_NODE: {
      symbol *ident = NULL;
      for (size_t i = 0; i < table->len; ++i) {
        symbol *sym = (symbol *)dyn_get(table, i);
        if (!strcmp(sym->ident, root->ident)) {
          ident = sym;
          break;
        }
      }

      assert(ident, "Undefined symbol.\n");

      fprintf(out, "\t%%%lu = load i32, i32* %%%lu, align 4\n", ssa,
              ident->loc);
      ++ssa;

    } break;

    default: break;
  }
}
