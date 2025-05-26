#pragma once

#include "libs/dynarray.h"
#include <stdio.h>

#define TOK_LIST                                                               \
  X(AMPER)                                                                     \
  X(AND)                                                                       \
  X(ARROW)                                                                     \
  X(AUTO)                                                                      \
  X(ASTERISK)                                                                  \
  X(BITOR)                                                                     \
  X(BREAK)                                                                     \
  X(CASE)                                                                      \
  X(CHAR)                                                                      \
  X(CHARLIT)                                                                   \
  X(COLON)                                                                     \
  X(COMMA)                                                                     \
  X(CONST)                                                                     \
  X(CONTINUE)                                                                  \
  X(DEFAULT)                                                                   \
  X(DECREM)                                                                    \
  X(DO)                                                                        \
  X(DOUBLE)                                                                    \
  X(DQUOTE)                                                                    \
  X(ELSE)                                                                      \
  X(ENUM)                                                                      \
  X(EQUALS)                                                                    \
  X(EQEQ)                                                                      \
  X(EXTERN)                                                                    \
  X(FLOAT)                                                                     \
  X(FLOATLIT)                                                                  \
  X(FOR)                                                                       \
  X(GE)                                                                        \
  X(GOTO)                                                                      \
  X(GT)                                                                        \
  X(HASH)                                                                      \
  X(IDENT)                                                                     \
  X(IF)                                                                        \
  X(INCREM)                                                                    \
  X(INLINE)                                                                    \
  X(INT)                                                                       \
  X(INTLIT)                                                                    \
  X(LBRACE)                                                                    \
  X(LBRACKET)                                                                  \
  X(LE)                                                                        \
  X(LONG)                                                                      \
  X(LPAREN)                                                                    \
  X(LT)                                                                        \
  X(LSHIFT)                                                                    \
  X(MINUS)                                                                     \
  X(NOT)                                                                       \
  X(NEQ)                                                                       \
  X(NOR)                                                                       \
  X(OR)                                                                        \
  X(PERIOD)                                                                    \
  X(PLUS)                                                                      \
  X(QUOTE)                                                                     \
  X(QUESTION)                                                                  \
  X(RBRACE)                                                                    \
  X(RBRACKET)                                                                  \
  X(REGISTER)                                                                  \
  X(RESTRICT)                                                                  \
  X(RETURN)                                                                    \
  X(RPAREN)                                                                    \
  X(RSHIFT)                                                                    \
  X(SEMI)                                                                      \
  X(SHORT)                                                                     \
  X(SIGNED)                                                                    \
  X(SIZEOF)                                                                    \
  X(SLASH)                                                                     \
  X(STATIC)                                                                    \
  X(STRINGLIT)                                                                 \
  X(STRUCT)                                                                    \
  X(SWITCH)                                                                    \
  X(TILDE)                                                                     \
  X(TYPEDEF)                                                                   \
  X(UNION)                                                                     \
  X(UNSIGNED)                                                                  \
  X(VOID)                                                                      \
  X(VOLATILE)                                                                  \
  X(WHILE)

#define X(name) name,
typedef enum { TOK_LIST } TokenType;
#undef X

#define X(name)                                                                \
  case name:                                                                   \
    return #name;
static const char *TOK2STR(TokenType t) {
  switch (t) {
    TOK_LIST
  default:
    return "UNKNOWN";
  }
}
#undef X

typedef struct {
  TokenType type;
  size_t start;
} Token;

void tokenize(char *buf, dyn_array *toks, size_t len);

static void dump(dyn_array *toks) {
  for (size_t i = 0; i < toks->len; ++i) {
    Token *t = (Token *)dyn_get(toks, i);
    printf("%s ", TOK2STR(t->type));
  }
  printf("\n\n");
}
