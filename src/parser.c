#include "parser.h"
#include "tokens.h"
#include "utils/ast.h"
#include "utils/dynarray.h"
#include "utils/str.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define error_expected(_m)                                                     \
  {                                                                            \
    Token *curr = (Token *)dyn_get(toks, (*i));                                \
    fprintf(stderr, "Expected %s on line %lu\n", _m,                           \
            getLineNo(buf, buf.len, curr->start));                             \
    return NULL;                                                               \
  }

#define current_token() (Token *)dyn_get(toks, (*i))
#define peek() (Token *)dyn_get(toks, (*i) + 1);
#define consume() (Token *)dyn_get(toks, (*i)++)
#define consume_discard() ++(*i)

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
         type == DOUBLE || type == LONG || type == VOID;
}

bool isNumberLiteral(TokenType type) {
  return type == INTLIT || type == FLOATLIT;
}

ast_node *try_parse_param(str buf, arena_t alloc, dyn_array *toks, size_t *i) {
  Token *front = consume();

  if (!isType(front->type))
    error_expected("type");

  TokenType type = front->type;

  front = consume();
  if (front->type != IDENT)
    error_expected("identifier");

  return create_param(type, parseString(buf, front->start).chars);
}

ast_node *try_parse_factor(str buf, arena_t alloc, dyn_array *toks, size_t *i) {
  Token *front = consume();

  if (front->type == PLUS || front->type == MINUS) {
    ast_node *atom = NULL;
    if (!(atom = try_parse_factor(buf, alloc, toks, i)))
      error_expected("atomic expression");

    return create_unop(atom, (front->type == MINUS) ? NUM_NEG : NUM_POS);

  } else if (isNumberLiteral(front->type))
    return create_num(parseNum(buf, front->start));

  else if (front->type == IDENT) {
    Token *next = current_token();
    if (next->type != LPAREN)
      return create_ident(parseString(buf, front->start).chars);

    consume_discard();

    ast_node *func = create_funccall(parseString(buf, front->start).chars);

    front = current_token();
    ast_node *expr = NULL;
    if (front->type != RPAREN && (expr = try_parse_expr(buf, alloc, toks, i))) {
      dyn_push(func->ast_func_call.args, expr);

      front = consume();
      while (front->type == COMMA &&
             (expr = try_parse_expr(buf, alloc, toks, i))) {
        dyn_push(func->ast_func_call.args, expr);
        front = current_token();
      }
    }

    if (front->type != RPAREN)
      error_expected("\')\'");

    consume_discard();

    return func;
  }

  else if (front->type == LPAREN) {
    ast_node *expr = NULL;
    if (!(expr = try_parse_expr(buf, alloc, toks, i)))
      error_expected("expression");

    front = consume();
    if (front->type != RPAREN)
      error_expected("\')\'");

    return expr;
  }

  return NULL;
}

ast_node *try_parse_term(str buf, arena_t alloc, dyn_array *toks, size_t *i) {
  ast_node *lhs = NULL;
  if (!(lhs = try_parse_factor(buf, alloc, toks, i)))
    error_expected("factor expression");

  Token *front = current_token();
  while (front->type == ASTERISK || front->type == SLASH) {
    consume_discard();
    ast_node *rhs = NULL;
    if (!(rhs = try_parse_factor(buf, alloc, toks, i)))
      error_expected("factor expression");

    lhs = create_binop(lhs, rhs, (front->type == ASTERISK) ? OP_TIMES : OP_DIV);

    front = current_token();
  }

  return lhs;
}

ast_node *try_parse_expr(str buf, arena_t alloc, dyn_array *toks, size_t *i) {
  ast_node *lhs = NULL;
  if (!(lhs = try_parse_term(buf, alloc, toks, i)))
    error_expected("factor expression");

  Token *front = current_token();
  while (front->type == PLUS || front->type == MINUS) {
    consume_discard();
    ast_node *rhs = NULL;
    if (!(rhs = try_parse_term(buf, alloc, toks, i)))
      error_expected("factor expression");

    lhs = create_binop(lhs, rhs, (front->type == PLUS) ? OP_PLUS : OP_MINUS);

    front = current_token();
  }

  return lhs;
}

ast_node *try_parse_stmt(str buf, arena_t alloc, dyn_array *toks, size_t *i) {
  Token *front = consume();

  ast_node *stmt = NULL;
  if (isType(front->type)) {
    front = consume();
    if (front->type != IDENT)
      error_expected("identifier");

    str ident = parseString(buf, front->start);

    front = consume();
    if (front->type != EQUALS)
      error_expected("\'=\'");

    ast_node *expr = NULL;
    if (!(expr = try_parse_expr(buf, alloc, toks, i)))
      error_expected("expression");

    front = consume();
    if (front->type != SEMI)
      error_expected("\';\'");

    stmt = create_stmt(VAR_DECL, ident.chars, expr);
  } else if (front->type == RETURN) {
    ast_node *expr = NULL;
    if (!(expr = try_parse_expr(buf, alloc, toks, i)))
      error_expected("expression");

    front = consume();
    if (front->type != SEMI)
      error_expected("\';\'");

    stmt = create_stmt(STMT_RET, "", expr);
  }

  return stmt;
}

ast_node *try_parse_funcdecl(str buf, arena_t alloc, dyn_array *toks,
                             size_t *i) {
  Token *front = consume();

  if (!isType(front->type))
    error_expected("type");

  TokenType ret_type = front->type;

  front = consume();
  if (front->type != IDENT)
    error_expected("identifier");

  str ident = parseString(buf, front->start);
  ast_node *func = create_funcdecl(ret_type, ident.chars);

  front = consume();
  if (front->type != LPAREN)
    error_expected("\'(\'");

  front = current_token();
  ast_node *param = NULL;
  if (front->type != RPAREN && (param = try_parse_param(buf, alloc, toks, i))) {
    dyn_push(func->ast_func_decl.params, param);

    front = consume();
    while (front->type == COMMA &&
           (param = try_parse_param(buf, alloc, toks, i))) {
      dyn_push(func->ast_func_decl.params, param);
      front = current_token();
    }
  }

  if (front->type != RPAREN)
    error_expected("\')\'");

  consume_discard();

  front = consume();
  if (front->type != LBRACE)
    error_expected("\'{\'");

  ast_node *stmt = NULL;
  while ((stmt = try_parse_stmt(buf, alloc, toks, i))) {
    dyn_push(func->ast_func_decl.stmts, stmt);

    front = current_token();
    if (front->type == RBRACE)
      break;
  }

  consume_discard();

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

  ast_traverse(*ast);

  arena_destroy(&alloc);
}
