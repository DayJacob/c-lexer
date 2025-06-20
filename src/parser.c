#include "parser.h"
#include "tokens.h"
#include "utils/ast.h"
#include "utils/dynarray.h"
#include "utils/str.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

dyn_array *table;
static size_t i = 0;

// Macro for handling errors
#define error_expected(_m)                                                     \
  {                                                                            \
    Token *curr = (Token *)dyn_get(toks, i);                                   \
    fprintf(stderr, "Expected %s on line %lu\n", _m,                           \
            getLineNo(buf, buf.len, curr->start));                             \
    return NULL;                                                               \
  }

// Useful macros for accessing tokens
#define current_token() (Token *)dyn_get(toks, i)
#define peek() (Token *)dyn_get(toks, i + 1);
#define peek_n(n) (Token *)dyn_get(toks, i + 1 + n);
#define consume() (Token *)dyn_get(toks, i++)
#define consume_discard() ++i

// Retrieves the symbol in the symbol table with the respective name.
Symbol *findInSymTable(const char *ident) {
  for (size_t i = 0; i < table->len; ++i) {
    Symbol *sym = (Symbol *)table->el[i];
    if (!strcmp(ident, sym->ident))
      return sym;
  }

  return NULL;
}

// Parses the string starting at position i and returns the numeric value
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

// Parses the string starting at position i and returns the string value
str parseString(str buf, size_t i) {
  str parsed = {0};
  while (isalnum(at(buf, i)) || at(buf, i) == '_') {
    ++i;
    ++parsed.len;
  }

  parsed.chars = dupl(buf, i - parsed.len, parsed.len);

  return parsed;
}

// Returns true if type is a valid C type, and false otherwise
// TODO: Add support for pointer types
inline bool isType(TokenType type) {
  return type == INT || type == FLOAT || type == CHAR || type == SHORT ||
         type == DOUBLE || type == LONG || type == VOID;
}

// Returns true if type is a valid C numeric literal, and false otherwise
inline bool isNumberLiteral(TokenType type) {
  return type == INTLIT || type == FLOATLIT;
}

// PARSING METHODS
// See grammar.bnf for the actual grammar rules and specifications needed to
// parse the tokens array.

ast_node *try_parse_param(str buf, dyn_array *toks) {
  Token *front = consume();

  if (!isType(front->type))
    error_expected("type");

  TokenType type = front->type;

  front = consume();
  if (front->type != IDENT)
    error_expected("identifier");

  Symbol *sym = (Symbol *)malloc(sizeof(Symbol));
  char *ident = parseString(buf, front->start).chars;
  sym->ident = ident;
  sym->type = type;
  dyn_push(table, sym);

  return create_param(type, ident);
}

ast_node *try_parse_factor(str buf, dyn_array *toks) {
  Token *front = consume();

  if (front->type == PLUS || front->type == MINUS) {
    ast_node *atom = NULL;
    if (!(atom = try_parse_factor(buf, toks)))
      error_expected("atomic expression");

    return create_unop(atom, (front->type == MINUS) ? NUM_NEG : NUM_POS);

  } else if (isNumberLiteral(front->type))
    return create_num(parseNum(buf, front->start), front->type);

  else if (front->type == IDENT) {
    Token *next = current_token();
    if (next->type != LPAREN) {
      char *ident = parseString(buf, front->start).chars;
      Symbol *sym = findInSymTable(ident);
      assert(sym, "Symbol not declared in scope.");

      return create_ident(ident, sym->type);
    }

    consume_discard();

    char *ident = parseString(buf, front->start).chars;
    Symbol *sym = findInSymTable(ident);
    assert(sym, "Call to undeclared function.\n");

    ast_node *func = create_funccall(ident, sym->type);

    front = current_token();
    ast_node *expr = NULL;
    if (front->type != RPAREN && (expr = try_parse_expr(buf, toks))) {
      dyn_push(func->ast_func_call.args, expr);

      front = consume();
      while (front->type == COMMA && (expr = try_parse_expr(buf, toks))) {
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
    if (!(expr = try_parse_expr(buf, toks)))
      error_expected("expression");

    front = consume();
    if (front->type != RPAREN)
      error_expected("\')\'");

    return expr;
  }

  return NULL;
}

ast_node *try_parse_term(str buf, dyn_array *toks) {
  ast_node *lhs = NULL;
  if (!(lhs = try_parse_factor(buf, toks)))
    error_expected("factor expression");

  Token *front = current_token();
  while (front->type == ASTERISK || front->type == SLASH) {
    consume_discard();
    ast_node *rhs = NULL;
    if (!(rhs = try_parse_factor(buf, toks)))
      error_expected("factor expression");

    lhs = create_binop(lhs, rhs, (front->type == ASTERISK) ? OP_TIMES : OP_DIV);

    front = current_token();
  }

  return lhs;
}

ast_node *try_parse_cond(str buf, dyn_array *toks) {
  ast_node *lhs = NULL;
  if (!(lhs = try_parse_term(buf, toks)))
    error_expected("terminal expression");

  Token *front = current_token();
  while (front->type == PLUS || front->type == MINUS) {
    consume_discard();
    ast_node *rhs = NULL;
    if (!(rhs = try_parse_term(buf, toks)))
      error_expected("terminal expression");

    lhs = create_binop(lhs, rhs, (front->type == PLUS) ? OP_PLUS : OP_MINUS);

    front = current_token();
  }

  return lhs;
}

ast_node *try_parse_equality(str buf, dyn_array *toks) {
  ast_node *lhs = NULL;
  if (!(lhs = try_parse_cond(buf, toks)))
    error_expected("conditional expression");

  Token *front = current_token();
  while (front->type == GE || front->type == GT || front->type == LE ||
         front->type == LT) {
    consume_discard();
    ast_node *rhs = NULL;
    if (!(rhs = try_parse_cond(buf, toks)))
      error_expected("conditional expression");

    BinOpType op;
    switch (front->type) {
      case GE: op = OP_GE; break;
      case GT: op = OP_GT; break;
      case LE: op = OP_LE; break;
      case LT: op = OP_LT; break;
      default: error_expected("valid operator");
    }

    lhs = create_binop(lhs, rhs, op);

    front = current_token();
  }

  return lhs;
}

ast_node *try_parse_expr(str buf, dyn_array *toks) {
  ast_node *lhs = NULL;
  if (!(lhs = try_parse_equality(buf, toks)))
    error_expected("equality expression");

  Token *front = current_token();
  while (front->type == EQEQ || front->type == NEQ) {
    consume_discard();
    ast_node *rhs = NULL;
    if (!(rhs = try_parse_equality(buf, toks)))
      error_expected("equality expression");

    lhs = create_binop(lhs, rhs, (front->type == EQEQ) ? OP_EQEQ : OP_NEQ);

    front = current_token();
  }

  return lhs;
}

ast_node *try_parse_stmt(str buf, dyn_array *toks) {
  Token *front = current_token();

  ast_node *stmt = NULL;
  if (isType(front->type)) {
    TokenType value = front->type;
    consume_discard();

    front = consume();
    if (front->type != IDENT)
      error_expected("identifier");

    str ident = parseString(buf, front->start);

    front = consume();
    if (front->type == SEMI) {
      stmt = create_vardecl(value, ident.chars);
    } else if (front->type == EQUALS) {
      ast_node *expr = NULL;
      if (!(expr = try_parse_expr(buf, toks)))
        error_expected("expression");

      front = consume();
      if (front->type != SEMI)
        error_expected("\';\'");

      stmt = create_varassign(value, ident.chars, expr);

    } else
      error_expected("\';\' or \'=\'");

    Symbol *sym = (Symbol *)malloc(sizeof(Symbol));
    sym->type = value;
    sym->ident = ident.chars;
    dyn_push(table, sym);

  } else if (front->type == RETURN) {
    consume_discard();

    ast_node *expr = NULL;
    if (!(expr = try_parse_expr(buf, toks)))
      error_expected("expression");

    front = consume();
    if (front->type != SEMI)
      error_expected("\';\'");

    stmt = create_return(VOID, expr);

  } else if (front->type == IF) {
    consume_discard();

    front = consume();
    if (front->type != LPAREN)
      error_expected("\'(\'");

    ast_node *pred = NULL;
    if (!(pred = try_parse_expr(buf, toks)))
      error_expected("predicate");

    front = consume();
    if (front->type != RPAREN)
      error_expected("\')\'");

    ast_node *scope = NULL;
    if (!(scope = try_parse_stmt(buf, toks)))
      error_expected("scope or statement");

    stmt = create_if_stmt(pred, scope, NULL);

    front = current_token();
    ast_node *alt = NULL;
    if (front->type == ELSE) {
      consume_discard();

      if (!(alt = try_parse_stmt(buf, toks)))
        error_expected("else scope or statement");
    }

    stmt->ast_stmt.if_stmt.alt = alt;

  } else if (front->type == LBRACE) {
    if (!(stmt = try_parse_scope(buf, toks)))
      error_expected("scope");

  } else if (front->type == WHILE) {
    consume_discard();

    front = consume();
    if (front->type != LPAREN)
      error_expected("\'(\'");

    ast_node *pred = NULL;
    if (!(pred = try_parse_expr(buf, toks)))
      error_expected("predicate expression");

    front = consume();
    if (front->type != RPAREN)
      error_expected("\')\'");

    ast_node *scope = NULL;
    if (!(scope = try_parse_stmt(buf, toks)))
      error_expected("scope or statement");

    stmt = create_while_stmt(pred, scope);
  }

  return stmt;
}

ast_node *try_parse_scope(str buf, dyn_array *toks) {
  Token *front = consume();
  if (front->type != LBRACE)
    error_expected("\'{\'");

  ast_node *scope = create_scope();

  ast_node *stmt = NULL;
  while ((stmt = try_parse_stmt(buf, toks))) {
    dyn_push(scope->ast_stmt.scope.stmts, stmt);

    front = current_token();
    if (front->type == RBRACE) {
      consume_discard();
      break;
    }
  }

  return scope;
}

ast_node *try_parse_funcdecl(str buf, dyn_array *toks) {
  Token *front = consume();

  if (!isType(front->type))
    error_expected("type");

  TokenType ret_type = front->type;

  front = consume();
  if (front->type != IDENT)
    error_expected("identifier");

  str ident = parseString(buf, front->start);
  ast_node *func = create_funcdecl(ret_type, ident.chars, NULL);

  front = consume();
  if (front->type != LPAREN)
    error_expected("\'(\'");

  front = current_token();
  ast_node *param = NULL;
  if (front->type != RPAREN && (param = try_parse_param(buf, toks))) {
    dyn_push(func->ast_func_decl.params, param);

    front = current_token();

    if (front->type == COMMA) {
      consume_discard();
      while ((param = try_parse_param(buf, toks))) {
        dyn_push(func->ast_func_decl.params, param);

        front = current_token();
        if (front->type != COMMA)
          break;
        else
          consume_discard();
      }
    }
  }

  if (front->type != RPAREN)
    error_expected("\')\'");

  consume_discard();

  func->ast_func_decl.scope = try_parse_scope(buf, toks);

  Symbol *sym = (Symbol *)malloc(sizeof(Symbol));
  sym->ident = ident.chars;
  sym->type = ret_type;
  dyn_push(table, sym);

  return func;
}

ast_node *try_parse_prgm(str buf, dyn_array *toks) {
  ast_node *ast = create_prgm();

  while (i + 1 < toks->len) {
    ast_node *func = NULL;
    if ((func = try_parse_funcdecl(buf, toks))) {
      dyn_push(ast->ast_prgm.func_decls, func);

    } else {
      fprintf(stderr, "Tried to parse function declaration and failed.\n");
      break;
    }
  }

  return ast;
}

void parse(str buf, dyn_array *toks, ast_node **ast) {
  *ast = try_parse_prgm(buf, toks);
}
