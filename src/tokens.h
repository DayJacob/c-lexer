#pragma once

#include "utils/dynarray.h"
#include "utils/str.h"
#include <stdio.h>

#define TOK_LIST                                                               \
  X(AMPER)                                                                     \
  X(AND)                                                                       \
  X(ANDEQ)                                                                     \
  X(ARROW)                                                                     \
  X(AUTO)                                                                      \
  X(ASTERISK)                                                                  \
  X(BACKSLASH)                                                                 \
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
  X(DIVEQ)                                                                     \
  X(DO)                                                                        \
  X(DOUBLE)                                                                    \
  X(DQUOTE)                                                                    \
  X(ELSE)                                                                      \
  X(EMPTY)                                                                     \
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
  X(LSHIFTEQ)                                                                  \
  X(MINUS)                                                                     \
  X(MINUSEQ)                                                                   \
  X(MODEQ)                                                                     \
  X(MODULO)                                                                    \
  X(NOT)                                                                       \
  X(NEQ)                                                                       \
  X(OR)                                                                        \
  X(OREQ)                                                                      \
  X(PERIOD)                                                                    \
  X(PLUS)                                                                      \
  X(PLUSEQ)                                                                    \
  X(QUOTE)                                                                     \
  X(QUESTION)                                                                  \
  X(RBRACE)                                                                    \
  X(RBRACKET)                                                                  \
  X(REGISTER)                                                                  \
  X(RESTRICT)                                                                  \
  X(RETURN)                                                                    \
  X(RPAREN)                                                                    \
  X(RSHIFT)                                                                    \
  X(RSHIFTEQ)                                                                  \
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
  X(TIMESEQ)                                                                   \
  X(TYPEDEF)                                                                   \
  X(UNION)                                                                     \
  X(UNSIGNED)                                                                  \
  X(VOID)                                                                      \
  X(VOLATILE)                                                                  \
  X(WHILE)                                                                     \
  X(XOR)                                                                       \
  X(XOREQ)

#define X(name) name,
typedef enum { TOK_LIST } TokenType;
#undef X

#define X(name)                                                                \
  case name: return #name;
static const char *TOK2STR(TokenType t) {
  switch (t) {
    TOK_LIST
    default: return "UNKNOWN";
  }
}
#undef X

typedef struct {
  TokenType type;
  size_t start;
} Token;

void tokenize(str buf, dyn_array *toks, size_t len);
void freeTokens(dyn_array *toks);

// Prints out the string-converted values of all the Tokens in the toks array.
static inline void dump(dyn_array *toks) {
  for (size_t i = 0; i < toks->len; ++i) {
    Token *t = (Token *)dyn_get(toks, i);
    printf("%s ", TOK2STR(t->type));
  }
  printf("\n\n");
}

size_t getLineNo(str buf, size_t len, size_t pos);
