#include "codegen.h"
#include "utils/ast.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static size_t ssa = 0;
static TokenType ret_type;

const char *asLLVMType(TokenType type) {
  switch (type) {
    case CHAR:   return "i8";
    case SHORT:  return "i16";
    case INT:    return "i32";
    case LONG:   return "i64";
    case VOID:   return "void";
    case DOUBLE: return "double";
    case FLOAT:  return "float";
    default:     return "";
  }
}

TokenType asBasicType(TokenType type) {
  switch (type) {
    case CHAR:
    case SHORT:
    case INT:
    case LONG:   return INT;
    case DOUBLE:
    case FLOAT:  return FLOAT;
    default:     return EMPTY;
  }
}

const size_t getAlignment(TokenType type) {
  switch (type) {
    case CHAR:   return 1;
    case SHORT:  return 2;
    case INT:
    case FLOAT:  return 4;
    case LONG:
    case DOUBLE: return 8;
    default:     return 0;
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

    default:        return false;
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

        case OP_EQEQ:  return left == right;
        case OP_NEQ:   return left != right;

        case OP_GE:    return left >= right;
        case OP_GT:    return left > right;
        case OP_LE:    return left <= right;
        case OP_LT:    return left < right;
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

symbol *findInSymTable(dyn_array *table, const char *ident) {
  for (size_t i = 0; i < table->len; ++i) {
    symbol *sym = (symbol *)table->el[i];
    if (!strcmp(ident, sym->ident))
      return sym;
  }

  return NULL;
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
      ret_type = root->value;
      fprintf(out, "define %s @%s(", asLLVMType(ret_type),
              root->ast_func_decl.ident);

      for (size_t i = 0; i < root->ast_func_decl.params->len; ++i) {
        ast_node *param = (ast_node *)dyn_get(root->ast_func_decl.params, i);
        symbol *sym = (symbol *)malloc(sizeof(symbol));
        sym->ident = param->ast_param.ident;
        sym->loc = ssa++;
        sym->type = param->value;

        fprintf(out, "%s noundef %%%lu", asLLVMType(sym->type), sym->loc);
        dyn_push(table, sym);

        if (i + 1 == root->ast_func_decl.params->len)
          break;

        fprintf(out, ", ");
      }

      ++ssa;
      fprintf(out, ") {\n");

      for (size_t i = 0; i < root->ast_func_decl.params->len; ++i) {
        ast_node *param = (ast_node *)dyn_get(root->ast_func_decl.params, i);
        symbol *sym = findInSymTable(table, param->ast_param.ident);

        fprintf(out, "\t%%%lu = alloca %s, align %lu\n", ssa,
                asLLVMType(sym->type), getAlignment(sym->type));
        fprintf(out, "\tstore %s %%%lu, ptr %%%lu, align %lu\n",
                asLLVMType(sym->type), sym->loc, ssa, getAlignment(sym->type));
        sym->loc = ssa;
        ++ssa;
      }

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
          var->type = root->value;
          dyn_push(table, var);

          fprintf(out, "\t%%%lu = alloca %s, align 4\n", ssa++,
                  asLLVMType(var->type));

          if (isComptimeExpr(root->ast_stmt.expr)) {
            double num = eval_tree(root->ast_stmt.expr);
            fprintf(out, "\tstore i32 %i, ptr %%%lu, align 4\n", (int)num,
                    ssa - 1);

            break;
          }

          generate_llvm(root->ast_stmt.expr, table, out);

        } break;

        case STMT_RET: {
          if (isComptimeExpr(root->ast_stmt.expr)) {
            double num = eval_tree(root->ast_stmt.expr);
            fprintf(out, "\tret %s ", asLLVMType(ret_type));

            switch (ret_type) {
              case CHAR:   fprintf(out, "%i", (char)num); break;
              case SHORT:  fprintf(out, "%i", (short)num); break;
              case INT:    fprintf(out, "%i", (int)num); break;
              case LONG:   fprintf(out, "%li", (long)num); break;
              case FLOAT:  fprintf(out, "%f", (float)num); break;
              case DOUBLE: fprintf(out, "%lf", (double)num); break;
              default:     break;
            }

            fprintf(out, "\n");

            break;
          }

          generate_llvm(root->ast_stmt.expr, table, out);

          if (asBasicType(ret_type) !=
              asBasicType(root->ast_stmt.expr->value)) {
            switch (asBasicType(ret_type)) {
              case INT:   fprintf(out, "\t%%%lu = fptosi ", ssa); break;
              case FLOAT: fprintf(out, "\t%%%lu = sitofp ", ssa); break;
              default:    break;
            }

            fprintf(out, "%s %%%lu to %s\n",
                    asLLVMType(root->ast_stmt.expr->value), ssa - 1,
                    asLLVMType(ret_type));
            ++ssa;

          } else if (getAlignment(ret_type) <
                     getAlignment(root->ast_stmt.expr->value)) {
            switch (asBasicType(ret_type)) {
              case INT:   fprintf(out, "\t%%%lu = trunc ", ssa); break;
              case FLOAT: fprintf(out, "\t%%%lu = fptrunc ", ssa); break;
              default:    break;
            }

            fprintf(out, "%s %%%lu to %s\n",
                    asLLVMType(root->ast_stmt.expr->value), ssa - 1,
                    asLLVMType(ret_type));
            ++ssa;

          } else if (getAlignment(ret_type) >
                     getAlignment(root->ast_stmt.expr->value)) {
            switch (asBasicType(ret_type)) {
              case INT:   fprintf(out, "\t%%%lu = sext ", ssa); break;
              case FLOAT: fprintf(out, "\t%%%lu = fpext ", ssa); break;
              default:    break;
            }

            fprintf(out, "%s %%%lu to %s\n",
                    asLLVMType(root->ast_stmt.expr->value), ssa - 1,
                    asLLVMType(ret_type));
            ++ssa;
          }

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
          fprintf(out, "%li, ",
                  (lhs_comptime ? lhs : ssa - (2 - (int)has_comptime_expr)));

          if (!rhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%li, align 4\n", (rhs_comptime ? rhs : ssa - 1));

          ++ssa;

        } break;

        case OP_MINUS: {
          fprintf(out, "\t%%%lu = sub nsw i32 ", ssa);

          if (!lhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%li, ",
                  (lhs_comptime ? lhs : ssa - (2 - (int)has_comptime_expr)));

          if (!rhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%li, align 4\n", (rhs_comptime ? rhs : ssa - 1));

          ++ssa;

        } break;

        case OP_TIMES: {
          fprintf(out, "\t%%%lu = mul nsw i32 ", ssa);

          if (!lhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%li, ",
                  (lhs_comptime ? lhs : ssa - (2 - (int)has_comptime_expr)));

          if (!rhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%li, align 4\n", (rhs_comptime ? rhs : ssa - 1));

          ++ssa;

        } break;

        case OP_DIV: {
          fprintf(out, "\t%%%lu = sdiv i32 ", ssa);

          if (!lhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%li, ",
                  (lhs_comptime ? lhs : ssa - (2 - (int)has_comptime_expr)));

          if (!rhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%li, align 4\n", (rhs_comptime ? rhs : ssa - 1));

          ++ssa;

        } break;

        default: break;
      }

    } break;

    case EXPR_UNOP: {
      generate_llvm(root->ast_unary_op.right, table, out);

      switch (root->ast_unary_op.type) {
        case NUM_NEG: {
          fprintf(out, "\t%%%lu = sub nsw 0, i32 %%%lu\n", ssa, ssa - 1);
          ++ssa;

        } break;

        default: break;
      }

    } break;

    case IDENT_NODE: {
      symbol *ident = findInSymTable(table, root->ident);
      assert(ident, "Undefined symbol.\n");

      fprintf(out, "\t%%%lu = load %s, ptr %%%lu, align %lu\n", ssa,
              asLLVMType(ident->type), ident->loc, getAlignment(ident->type));
      ++ssa;

      root->value = ident->type;

    } break;

    default: break;
  }
}
