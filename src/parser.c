#include "parser.h"
#include "tokens.h"
#include "utils/arena.h"
#include "utils/ast.h"
#include "utils/dynarray.h"
#include "utils/str.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define error_expected(_m)                                                     \
  {                                                                            \
    fprintf(stderr, "Expected %s on line %lu\n", _m,                           \
            getLineNo(buf, buf.len, *i));                                      \
    return NULL;                                                               \
  }

double parseNum(str buf, size_t i) {
  size_t len = 0;
  while (isdigit(at(buf, i)) || at(buf, i) == '.') {
    ++i;
    ++len;
  }

  char *dst = (char *)malloc(len);
  strncpy(dst, buf.chars + (i - len), len);

  double result = atof(dst);
  free(dst);
  return result;
}

str parseString(str buf, size_t i) {
  str parsed = {0};
  while (isalnum(at(buf, i)) || at(buf, i) == '_') {
    ++i;
    ++parsed.len;
  }

  parsed.chars = dupl(buf, i - parsed.len, parsed.len);

  return parsed;
}

bool isType(TokenType type) {
  return type == INT || type == FLOAT || type == CHAR || type == SHORT ||
         type == DOUBLE || type == LONG;
}

bool isNumberLiteral(TokenType type) {
  return type == INTLIT || type == FLOATLIT;
}

ast_node *try_parse_expr(str buf, arena_t alloc, dyn_array *toks, size_t *i) {
  Token *front = (Token *)dyn_get(toks, (*i)++);

  if (isNumberLiteral(front->type))
    return create_num(parseNum(buf, *i));

  return NULL;
}

ast_node *try_parse_stmt(str buf, arena_t alloc, dyn_array *toks, size_t *i) {
  Token *front = (Token *)dyn_get(toks, (*i)++);

  ast_node *stmt = NULL;
  if (isType(front->type)) {
    front = (Token *)dyn_get(toks, (*i)++);
    if (front->type != IDENT)
      error_expected("identifier");

    str ident = parseString(buf, *i);

    front = (Token *)dyn_get(toks, (*i)++);
    if (front->type != EQUALS)
      error_expected("\'=\'");

    ast_node *expr = NULL;
    if (!(expr = try_parse_expr(buf, alloc, toks, i)))
      error_expected("expression");

    front = (Token *)dyn_get(toks, (*i)++);
    if (front->type != SEMI)
      error_expected("\';\'");

    stmt = create_stmt(VAR_DECL, ident.chars, expr);
  } else if (front->type == RETURN) {
    ast_node *expr = NULL;
    if (!(expr = try_parse_expr(buf, alloc, toks, i)))
      error_expected("expression");

    front = (Token *)dyn_get(toks, (*i)++);
    if (front->type != SEMI)
      error_expected("\';\'");

    stmt = create_stmt(STMT_RET, "", expr);
  }

  return stmt;
}

ast_node *try_parse_funcdecl(str buf, arena_t alloc, dyn_array *toks,
                             size_t *i) {
  Token *front = (Token *)dyn_get(toks, (*i)++);
  Token *next = NULL;

  if (!isType(front->type))
    error_expected("type");

  TokenType ret_type = front->type;

  front = (Token *)dyn_get(toks, (*i)++);
  if (front->type != IDENT)
    error_expected("identifier");

  str ident = parseString(buf, *i);

  front = (Token *)dyn_get(toks, (*i)++);
  next = (Token *)dyn_get(toks, (*i)++);
  // TODO: Add support for function parameters
  if (front->type != LPAREN || next->type != RPAREN)
    error_expected("\'()\'");

  front = (Token *)dyn_get(toks, (*i)++);
  if (front->type != LBRACE)
    error_expected("\'{\'");

  ast_node *func = create_funcdecl(ret_type, ident.chars);

  ast_node *stmt = NULL;
  while ((stmt = try_parse_stmt(buf, alloc, toks, i))) {
    dyn_push(func->ast_func_decl.stmts, stmt);

    front = (Token *)dyn_get(toks, *i);
    if (front->type == RBRACE)
      break;
  }

  ++(*i);

  return func;
}

ast_node *try_parse_prgm(str buf, arena_t alloc, dyn_array *toks) {
  ast_node *ast = create_prgm();

  size_t i = 0;
  while (i < toks->len) {
    ast_node *func = NULL;
    if ((func = try_parse_funcdecl(buf, alloc, toks, &i))) {
      dyn_push(ast->ast_prgm.func_decls, func);
    } else {
      fprintf(stderr, "Tried to parse function declaration and failed.\n");
      break;
    }
  }

  return ast;
}

void parse(str buf, dyn_array *toks, ast_node **ast) {
  arena_t alloc = arena_init(1024 * 4);

  *ast = try_parse_prgm(buf, alloc, toks);

  arena_destroy(&alloc);
}
