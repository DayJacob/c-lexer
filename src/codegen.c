#include "codegen.h"
#include "parser.h"
#include "utils/ast.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static size_t ssa = 0;
static TokenType ret_type;

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

void generate_llvm(ast_node *root, FILE *out) {
  if (!root)
    return;

  switch (root->type) {

    case PRGM: {
      for (size_t i = 0; i < root->ast_prgm.func_decls->len; ++i)
        generate_llvm((ast_node *)root->ast_prgm.func_decls->el[i], out);

    } break;

    case FUNC_DECL: {
      ret_type = root->value;
      fprintf(out, "define %s @%s(", asLLVMType(ret_type),
              root->ast_func_decl.ident);

      for (size_t i = 0; i < root->ast_func_decl.params->len; ++i) {
        ast_node *param = (ast_node *)root->ast_func_decl.params->el[i];
        Symbol *sym = findInSymTable(param->ast_param.ident);
        sym->loc = ssa++;

        fprintf(out, "%s noundef %%%lu", asLLVMType(sym->type), sym->loc);

        if (i + 1 == root->ast_func_decl.params->len)
          break;

        fprintf(out, ", ");
      }

      ++ssa;
      fprintf(out, ") {\n");

      fprintf(out, "\t%%%lu = alloca %s, align %lu\n", ssa++,
              asLLVMType(ret_type), getAlignment(ret_type));

      for (size_t i = 0; i < root->ast_func_decl.params->len; ++i) {
        ast_node *param = (ast_node *)root->ast_func_decl.params->el[i];
        Symbol *sym = findInSymTable(param->ast_param.ident);

        fprintf(out, "\t%%%lu = alloca %s, align %lu\n", ssa,
                asLLVMType(sym->type), getAlignment(sym->type));
        fprintf(out, "\tstore %s %%%lu, ptr %%%lu, align %lu\n",
                asLLVMType(sym->type), sym->loc, ssa, getAlignment(sym->type));
        sym->loc = ssa;
        ++ssa;
      }

      generate_llvm(root->ast_func_decl.scope, out);

      fprintf(out, "}\n\n");

    } break;

    case SCOPE: {
      for (size_t i = 0; i < root->ast_scope.stmts->len; ++i)
        generate_llvm((ast_node *)root->ast_scope.stmts->el[i], out);

    } break;

    case STMT: {
      switch (root->ast_stmt.type) {

        case VAR_DECL: {
          Symbol *var = findInSymTable(root->ast_stmt.ident);
          var->loc = ssa;

          fprintf(out, "\t%%%lu = alloca %s, align %lu\n", ssa++,
                  asLLVMType(var->type), getAlignment(var->type));

          if (isComptimeExpr(root->ast_stmt.expr)) {
            double num = eval_tree(root->ast_stmt.expr);
            fprintf(out, "\tstore i32 %i, ptr %%%lu, align 4\n", (int)num,
                    ssa - 1);

            break;
          }

          generate_llvm(root->ast_stmt.expr, out);

          fprintf(out, "\tstore %s %%%lu, ptr %%%lu, align %lu\n",
                  asLLVMType(root->value), ssa - 1, var->loc,
                  getAlignment(root->value));

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

          generate_llvm(root->ast_stmt.expr, out);

          fprintf(out, "\tret %s %%%lu\n", asLLVMType(ret_type), ssa - 1);

        } break;

        default: break;
      }

    } break;

    case IF_STMT: {
      generate_llvm(root->ast_if_stmt.pred, out);

      fprintf(out, "\tbr i1 %%%lu, label %%then, label %%%s\n", ssa - 1,
              (root->ast_if_stmt.alt) ? "else" : "after");
      fprintf(out, "\nthen:\n");
      ++ssa;

      generate_llvm(root->ast_if_stmt.scope, out);

      if (root->ast_if_stmt.alt) {
        fprintf(out, "\nelse:\n");
        generate_llvm(root->ast_if_stmt.alt, out);
      }

      fprintf(out, "\nafter:\n");

    } break;

    case EXPR_BINOP: {
      size_t lhs, rhs;
      bool lhs_comptime = false, rhs_comptime = false;

      if ((lhs_comptime = isComptimeExpr(root->ast_binary_op.left)))
        lhs = (int)eval_tree(root->ast_binary_op.left);
      else
        generate_llvm(root->ast_binary_op.left, out);

      if ((rhs_comptime = isComptimeExpr(root->ast_binary_op.right)))
        rhs = (int)eval_tree(root->ast_binary_op.right);
      else
        generate_llvm(root->ast_binary_op.right, out);

      bool has_comptime_expr = lhs_comptime || rhs_comptime;

      switch (root->ast_binary_op.type) {

        case OP_PLUS: {
          if (asBasicType(root->value) == FLOAT)
            fprintf(out, "\t%%%lu = fadd %s ", ssa, asLLVMType(root->value));
          else
            fprintf(out, "\t%%%lu = add nsw %s ", ssa, asLLVMType(root->value));

          if (!lhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%li, ",
                  (lhs_comptime ? lhs : ssa - (2 - (int)has_comptime_expr)));

          if (!rhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%li\n", (rhs_comptime ? rhs : ssa - 1));

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
          fprintf(out, "%li\n", (rhs_comptime ? rhs : ssa - 1));

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
          fprintf(out, "%li\n", (rhs_comptime ? rhs : ssa - 1));

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
          fprintf(out, "%li\n", (rhs_comptime ? rhs : ssa - 1));

          ++ssa;

        } break;

        case OP_EQEQ: {
          fprintf(out, "\t%%%lu = icmp eq %s ", ssa, asLLVMType(root->value));

          if (!lhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%li, ",
                  (lhs_comptime ? lhs : ssa - (2 - (int)has_comptime_expr)));

          if (!rhs_comptime)
            fprintf(out, "%%");
          fprintf(out, "%li\n", (rhs_comptime ? rhs : ssa - 1));

          ++ssa;

        } break;

        default: break;
      }

    } break;

    case EXPR_UNOP: {
      generate_llvm(root->ast_unary_op.right, out);

      switch (root->ast_unary_op.type) {
        case NUM_NEG: {
          fprintf(out, "\t%%%lu = sub nsw 0, i32 %%%lu\n", ssa, ssa - 1);
          ++ssa;

        } break;

        case EXTEND: {
          switch (asBasicType(root->value)) {
            case INT:   fprintf(out, "\t%%%lu = sext ", ssa); break;
            case FLOAT: fprintf(out, "\t%%%lu = fpext ", ssa); break;
            default:    break;
          }

          fprintf(out, "%s %%%lu to %s\n",
                  asLLVMType(root->ast_unary_op.right->value), ssa - 1,
                  asLLVMType(root->value));
          ++ssa;
        } break;

        case TRUNC: {
          switch (asBasicType(root->value)) {
            case INT:   fprintf(out, "\t%%%lu = trunc ", ssa); break;
            case FLOAT: fprintf(out, "\t%%%lu = fptrunc ", ssa); break;
            default:    break;
          }

          fprintf(out, "%s %%%lu to %s\n",
                  asLLVMType(root->ast_unary_op.right->value), ssa - 1,
                  asLLVMType(root->value));
          ++ssa;
        } break;

        case INT_TOFLOAT: {
          fprintf(out, "\t%%%lu = sitofp %s %%%lu to %s\n", ssa,
                  asLLVMType(root->ast_unary_op.right->value), ssa - 1,
                  asLLVMType(root->value));
          ++ssa;
        } break;

        case FLOAT_TOINT: {
          fprintf(out, "\t%%%lu = fptosi %s %%%lu to %s\n", ssa,
                  asLLVMType(root->ast_unary_op.right->value), ssa - 1,
                  asLLVMType(root->value));
          ++ssa;
        } break;

        default: break;
      }

    } break;

    case IDENT_NODE: {
      Symbol *ident = findInSymTable(root->ident);
      assert(ident, "Identifier referenced before declaration.\n");

      fprintf(out, "\t%%%lu = load %s, ptr %%%lu, align %lu\n", ssa,
              asLLVMType(ident->type), ident->loc, getAlignment(ident->type));
      ++ssa;

      root->value = ident->type;

    } break;

    case FUNC_CALL: {
      Symbol *ident = findInSymTable(root->ast_func_call.ident);
      assert(ident, "Call to undefined function.\n");

      for (size_t i = 0; i < root->ast_func_call.args->len; ++i) {
        ast_node *expr = (ast_node *)root->ast_func_call.args->el[i];
        if (isComptimeExpr(expr)) {
          // double num = eval_tree(expr);
        }

        generate_llvm((ast_node *)root->ast_func_call.args->el[i], out);
      }

      fprintf(out, "\t%%%lu = call %s @%s(", ssa, asLLVMType(ident->type),
              ident->ident);
      ssa++;

      fprintf(out, ")\n");

    } break;

    default: break;
  }
}
