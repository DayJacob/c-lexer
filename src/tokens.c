#include "tokens.h"
#include "utils/dynarray.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

size_t getLineNo(str buf, size_t len, size_t pos) {
  assert(pos <= len, "Index out of bounds.");

  size_t line = 1;
  for (int i = 0; i <= pos; ++i) {
    if (at(buf, i) == '\n')
      ++line;
  }

  return line;
}

void tokenize(str buf, dyn_array *toks, size_t len) {
  size_t i = 0;

  while (i < len) {
    // ignore comments
    if (at(buf, i) == '/' && at(buf, i + 1) == '/') {
      while (at(buf, i++) != '\n')
        ;
    }

    // ignore whitespace
    else if (isspace(at(buf, i))) {
      ++i;
    }

    // char lit
    else if (at(buf, i) == '"') {
      ++i;

      size_t toksize = 1;
      while (at(buf, i++) != '"')
        ++toksize;

      Token *ptr = (Token *)malloc(sizeof(Token));
      ptr->type = STRINGLIT;
      ptr->start = i - toksize;

      dyn_push(toks, ptr);
    }

    else if (at(buf, i) == '\'') {
      ++i;

      size_t toksize = 1;
      while (at(buf, i++) != '\'')
        ++toksize;

      Token *ptr = (Token *)malloc(sizeof(Token));
      ptr->type = CHARLIT;
      ptr->start = i - toksize;

      dyn_push(toks, ptr);
    }

    // check special char
    else if (ispunct(at(buf, i))) {
      Token *ptr = (Token *)malloc(sizeof(Token));

      switch (at(buf, i)) {

      case '+': {
        if (at(buf, i + 1) == '+') {
          ptr->type = INCREM;
          ++i;
        } else if (at(buf, i + 1) == '=') {
          ptr->type = PLUSEQ;
          ++i;
        } else
          ptr->type = PLUS;

      } break;

      case '-': {
        if (at(buf, i + 1) == '-') {
          ptr->type = DECREM;
          ++i;
        } else if (at(buf, i + 1) == '>') {
          ptr->type = ARROW;
          ++i;
        } else if (at(buf, i + 1) == '=') {
          ptr->type = MINUSEQ;
          ++i;
        } else
          ptr->type = MINUS;

      } break;

      case '*': {
        if (at(buf, i + 1) == '=') {
          ptr->type = TIMESEQ;
          ++i;
        } else
          ptr->type = ASTERISK;

      } break;

      case '/': {
        if (at(buf, i + 1) == '=') {
          ptr->type = DIVEQ;
          ++i;
        } else
          ptr->type = SLASH;

      } break;

      case '%': {
        if (at(buf, i + 1) == '=') {
          ptr->type = MODEQ;
          ++i;
        } else
          ptr->type = MODULO;
      }

      case '{': {
        ptr->type = LBRACE;

      } break;

      case '}': {
        ptr->type = RBRACE;

      } break;

      case '[': {
        ptr->type = LBRACKET;

      } break;

      case ']': {
        ptr->type = RBRACKET;

      } break;

      case '(': {
        ptr->type = LPAREN;

      } break;

      case ')': {
        ptr->type = RPAREN;

      } break;

      case '=': {
        if (at(buf, i + 1) == '=') {
          ptr->type = EQEQ;
          ++i;
        } else
          ptr->type = EQUALS;

      } break;

      case ';': {
        ptr->type = SEMI;

      } break;

      case ':': {
        ptr->type = COLON;

      } break;

      case ',': {
        ptr->type = COMMA;

      } break;

      case '>': {
        if (at(buf, i + 1) == '=') {
          ptr->type = GE;
          ++i;
        } else if (at(buf, i + 1) == '>') {
          if (at(buf, i + 2) == '=') {
            ptr->type = RSHIFTEQ;
            ++i;
          } else
            ptr->type = RSHIFT;

          ++i;
        } else {
          ptr->type = GT;
        }
      } break;

      case '<': {
        if (at(buf, i + 1) == '=') {
          ptr->type = LE;
          ++i;
        } else if (at(buf, i + 1) == '<') {
          if (at(buf, i + 2) == '=') {
            ptr->type = LSHIFTEQ;
            ++i;
          } else
            ptr->type = LSHIFT;

          ++i;
        } else {
          ptr->type = LT;
        }
      } break;

      case '!': {
        if (at(buf, i + 1) == '=') {
          ptr->type = NEQ;
          ++i;
        } else
          ptr->type = NOT;

      } break;

      case '~': {
        ptr->type = TILDE;

      } break;

      case '.': {
        ptr->type = PERIOD;

      } break;

      case '#': {
        ptr->type = HASH;

      } break;

      case '&': {
        if (at(buf, i + 1) == '&') {
          ptr->type = AND;
          ++i;
        } else if (at(buf, i + 1) == '=') {
          ptr->type = ANDEQ;
          ++i;
        } else
          ptr->type = AMPER;

      } break;

      case '|': {
        if (at(buf, i + 1) == '|') {
          ptr->type = OR;
          ++i;
        } else if (at(buf, i + 1) == '=') {
          ptr->type = OREQ;
          ++i;
        } else
          ptr->type = BITOR;

      } break;

      case '^': {
        if (at(buf, i + 1) == '=') {
          ptr->type = XOREQ;
          ++i;
        } else
          ptr->type = XOR;

      } break;

      case '?': {
        ptr->type = QUESTION;

      } break;

      case '\\': {
        ptr->type = BACKSLASH;

      } break;

      default: {
        fprintf(stderr, "Unrecognized token %c (line %lu).\n", at(buf, i),
                getLineNo(buf, len, i));
        dyn_destroy(toks);
        exit(1);
      }
      }

      ptr->start = i;
      dyn_push(toks, ptr);

      ++i;
    }

    // alpha
    else if (isalpha(at(buf, i))) {
      size_t toksize = 0;
      while (isalnum(at(buf, i)) || at(buf, i) == '_') {
        ++i;
        ++toksize;
      }

      char *bufcmp = dupl(buf, i - toksize, toksize);

      Token *ptr = (Token *)malloc(sizeof(Token));

      if (!strncmp(bufcmp, "auto", toksize))
        ptr->type = AUTO;

      else if (!strncmp(bufcmp, "break", toksize))
        ptr->type = BREAK;

      else if (!strncmp(bufcmp, "case", toksize))
        ptr->type = CASE;

      else if (!strncmp(bufcmp, "char", toksize))
        ptr->type = CHAR;

      else if (!strncmp(bufcmp, "const", toksize))
        ptr->type = CONST;

      else if (!strncmp(bufcmp, "continue", toksize))
        ptr->type = CONTINUE;

      else if (!strncmp(bufcmp, "default", toksize))
        ptr->type = DEFAULT;

      else if (!strncmp(bufcmp, "do", toksize))
        ptr->type = DO;

      else if (!strncmp(bufcmp, "double", toksize))
        ptr->type = DOUBLE;

      else if (!strncmp(bufcmp, "else", toksize))
        ptr->type = ELSE;

      else if (!strncmp(bufcmp, "enum", toksize))
        ptr->type = ENUM;

      else if (!strncmp(bufcmp, "extern", toksize))
        ptr->type = EXTERN;

      else if (!strncmp(bufcmp, "float", toksize))
        ptr->type = FLOAT;

      else if (!strncmp(bufcmp, "for", toksize))
        ptr->type = FOR;

      else if (!strncmp(bufcmp, "goto", toksize))
        ptr->type = GOTO;

      else if (!strncmp(bufcmp, "if", toksize))
        ptr->type = IF;

      else if (!strncmp(bufcmp, "inline", toksize))
        ptr->type = INLINE;

      else if (!strncmp(bufcmp, "int", toksize))
        ptr->type = INT;

      else if (!strncmp(bufcmp, "long", toksize))
        ptr->type = LONG;

      else if (!strncmp(bufcmp, "register", toksize))
        ptr->type = REGISTER;

      else if (!strncmp(bufcmp, "restrict", toksize))
        ptr->type = RESTRICT;

      else if (!strncmp(bufcmp, "return", toksize))
        ptr->type = RETURN;

      else if (!strncmp(bufcmp, "short", toksize))
        ptr->type = SHORT;

      else if (!strncmp(bufcmp, "signed", toksize))
        ptr->type = SIGNED;

      else if (!strncmp(bufcmp, "sizeof", toksize))
        ptr->type = SIZEOF;

      else if (!strncmp(bufcmp, "static", toksize))
        ptr->type = STATIC;

      else if (!strncmp(bufcmp, "struct", toksize))
        ptr->type = STRUCT;

      else if (!strncmp(bufcmp, "switch", toksize))
        ptr->type = SWITCH;

      else if (!strncmp(bufcmp, "typedef", toksize))
        ptr->type = TYPEDEF;

      else if (!strncmp(bufcmp, "union", toksize))
        ptr->type = UNION;

      else if (!strncmp(bufcmp, "unsigned", toksize))
        ptr->type = UNSIGNED;

      else if (!strncmp(bufcmp, "void", toksize))
        ptr->type = VOID;

      else if (!strncmp(bufcmp, "volatile", toksize))
        ptr->type = VOLATILE;

      else if (!strncmp(bufcmp, "while", toksize))
        ptr->type = WHILE;

      else
        ptr->type = IDENT;

      ptr->start = i - toksize;

      dyn_push(toks, ptr);

      free(bufcmp);
      bufcmp = NULL;
    }

    // digit
    else if (isdigit(at(buf, i))) {
      size_t toksize = 0;
      bool isFloat = false;
      while (isdigit(at(buf, i)) || at(buf, i) == '.') {
        if (at(buf, i) == '.')
          isFloat = true;
        ++toksize;
        ++i;
      }

      Token *ptr = (Token *)malloc(sizeof(Token));

      if (isFloat)
        ptr->type = FLOATLIT;
      else
        ptr->type = INTLIT;

      ptr->start = i - toksize;
      dyn_push(toks, ptr);

    }

    // otherwise
    else {
      fprintf(stderr, "Unrecognized token %c (line %lu).\n", at(buf, i),
              getLineNo(buf, len, i));
      dyn_destroy(toks);
      exit(1);
    }
  }

  dump(toks);
}

void freeTokens(dyn_array *toks) {
  for (size_t i = 0; i < toks->len; ++i) {
    Token *t = (Token *)dyn_get(toks, i);
    free(t);
    t = NULL;
  }
}
