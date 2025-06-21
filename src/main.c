#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "analysis.h"
#include "codegen.h"
#include "parser.h"
#include "tokens.h"
#include "utils/ast.h"
#include "utils/dynarray.h"
#include "utils/str.h"

// Prints the AST to stdout
void printTree(ast_node *root) {
  if (!root)
    return;

  switch (root->type) {
    case PRGM: {
      printf("PRGM:\n");

      for (size_t i = 0; i < root->ast_prgm.func_decls->len; ++i)
        printTree((ast_node *)dyn_get(root->ast_prgm.func_decls, i));

    } break;

    case FUNC_DECL: {
      printf("FUNC:\n");

      printTree(root->ast_func_decl.scope);

    } break;

    case SCOPE: {
      for (size_t i = 0; i < root->ast_stmt.scope.stmts->len; ++i)
        printTree((ast_node *)dyn_get(root->ast_stmt.scope.stmts, i));

    } break;

    case STMT: {
      printf("STMT: ");

      switch (root->ast_stmt.type) {
        case RET_STMT:
          printf("RETURN\n");
          printTree(root->ast_stmt.ret.expr);
          break;

        case VAR_DECL:
          printf("VAR DECL %s\n", root->ast_stmt.ident_decl);
          break;

        case VAR_ASSIGN:
          printf("VAR ASSIGN %s\n", root->ast_stmt.var_assign.ident);
          printTree(root->ast_stmt.var_assign.expr);
          break;

        default: break;
      }

    } break;

    case EXPR_BINOP: {
      printf("BINOP: ");

      switch (root->ast_binary_op.type) {
        case OP_PLUS:  printf("+\n"); break;
        case OP_MINUS: printf("-\n"); break;
        case OP_TIMES: printf("*\n"); break;
        case OP_DIV:   printf("/\n"); break;
        default:       break;
      }

      printTree(root->ast_binary_op.left);
      printTree(root->ast_binary_op.right);

    } break;

    case EXPR_UNOP: {
      printf("UNOP: ");

      switch (root->ast_unary_op.type) {
        case OP_NEG:      printf("-\n"); break;
        case OP_POS:      printf("+\n"); break;
        case TRUNC:       printf("cast truncate\n"); break;
        case EXTEND:      printf("cast extend\n"); break;
        case FLOAT_TOINT: printf("cast float to int\n"); break;
        case INT_TOFLOAT: printf("cast int to float\n"); break;
        default:          break;
      }

      printTree(root->ast_unary_op.right);

    } break;

    default: break;
  }
}

int main(int argc, char *argv[]) {
  // CLI must take in one argument, potentially more later.
  if (argc != 2) {
    fprintf(stderr, "Fmt: ./main <file>\n");
    return EXIT_FAILURE;
  }

  // Attempt to open the file provided through CLI, panic on failure
  FILE *fp = fopen(argv[1], "r");
  if (!fp) {
    fprintf(stderr, "Could not open file\n");
    return EXIT_FAILURE;
  }

  // Get the file length in characters
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  // Allocate a buffer to read the file into, panic on failure
  char *buf = (char *)malloc(fsize + 1);
  if (!buf) {
    fprintf(stderr, "Memory alloc failed\n");
    fclose(fp);
    return EXIT_FAILURE;
  }

  // Read the file into the buffer, panic on failure
  if (fread((void *)buf, fsize, 1, fp) == fsize) {
    fprintf(stderr, "Could not read file into buffer\n");
    fclose(fp);
    free(buf);
    return EXIT_FAILURE;
  }

  // Initialize the arena allocator used for parsing
  arena_init(&alloc, 1024 * 1024 * 4);

  // Initialize the tokens array.
  dyn_array *toks = dyn_init(fsize / 10);
  ast_node *ast = NULL;

  // Initialize the Symbol table
  table = dyn_init(5);

  // Tokenize and parse the input
  tokenize((str){.len = fsize, .chars = buf}, toks, fsize);
  parse((str){.len = fsize, .chars = buf}, toks, &ast);
  analyze(ast);

  printTree(ast);
  printf("\n");

  // Attempt to open a file to write generated LLVM IR, panic on failure
  FILE *out = fopen("build/out.ll", "w");
  if (!fp) {
    fprintf(stderr, "Could not open file\n");
    fclose(fp);
    fclose(out);
    free(buf);
    freeTokens(toks);
    dyn_destroy(toks);
    ast_destroy(ast);
    arena_destroy(&alloc);
    return EXIT_FAILURE;
  }

  // Generate LLVM
  generate_llvm(ast, out);

  // Print Symbol table contents to stdout
  for (size_t i = 0; i < table->len; ++i) {
    Symbol *sym = (Symbol *)dyn_get(table, i);
    printf("Symbol: %s\tLocation: %lu\n", sym->ident, sym->loc);
    free(sym);
  }
  printf("\n\n");

  // Clean up table, tokens, AST, and arena allocator, and all file
  // pointers/buffers
  dyn_destroy(table);

  freeTokens(toks);

  dyn_destroy(toks);
  ast_destroy(ast);

  arena_destroy(&alloc);

  fclose(out);
  fclose(fp);
  free(buf);

  return EXIT_SUCCESS;
}
