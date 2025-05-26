#include "tokens.h"
#include "libs/dynarray.h"
#include <ctype.h>
#include <string.h>

void tokenize(char *buf, dyn_array *toks, size_t len) {
  size_t i = 0;

  while (i < len) {
    // ignore comments
    if (buf[i] == '/' && buf[i + 1] == '/') {
      while (buf[i++] != '\n')
        ;
    }

    // ignore whitespace
    else if (isspace(buf[i])) {
      ++i;
    }

    // char lit
    else if (buf[i] == '"') {
      ++i;

      size_t toksize = 1;
      while (buf[i++] != '"')
        ++toksize;

      Token *ptr = (Token *)malloc(sizeof(Token));
      ptr->type = STRINGLIT;
      ptr->start = i - toksize;

      dyn_push(toks, ptr);
    }

    // check special char
    else if (ispunct(buf[i])) {
      Token *ptr = (Token *)malloc(sizeof(Token));

      switch (buf[i]) {

      case '+': {
        if (buf[i + 1] == '+') {
          ptr->type = INCREM;
          ++i;
        } else
          ptr->type = PLUS;

      } break;

      case '-': {
        if (buf[i + 1] == '-') {
          ptr->type = DECREM;
          ++i;
        } else if (buf[i + 1] == '>') {
          ptr->type = ARROW;
          ++i;
        } else
          ptr->type = MINUS;

      } break;

      case '*': {
        ptr->type = ASTERISK;

      } break;

      case '/': {
        ptr->type = SLASH;

      } break;

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
        if (buf[i + 1] == '=') {
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
        if (buf[i + 1] == '=') {
          ptr->type = GE;
          ++i;
        } else if (buf[i + 1] == '>') {
          ptr->type = RSHIFT;
          ++i;
        } else {
          ptr->type = GT;
        }
      } break;

      case '<': {
        if (buf[i + 1] == '=') {
          ptr->type = LE;
          ++i;
        } else if (buf[i + 1] == '<') {
          ptr->type = LSHIFT;
          ++i;
        } else {
          ptr->type = LT;
        }
      } break;

      case '!': {
        if (buf[i + 1] == '=') {
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
        if (buf[i + 1] == '&') {
          ptr->type = AND;
          ++i;
        } else
          ptr->type = AMPER;

      } break;

      case '|': {
        if (buf[i + 1] == '|') {
          ptr->type = OR;
          ++i;
        } else
          ptr->type = BITOR;

      } break;

      case '^': {
        ptr->type = NOR;

      } break;

      default: {
        fprintf(stderr, "Unrecognized token %c.\n", buf[i]);
        dyn_destroy(toks);
        exit(1);
      }
      }

      ptr->start = i;
      dyn_push(toks, ptr);

      ++i;
    }

    // alpha
    else if (isalpha(buf[i])) {
      size_t toksize = 0;
      while (isalpha(buf[i++]))
        ++toksize;
      --i;

      char *bufcmp = strndup((const char *)&buf[i - toksize], toksize);

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

    }

    // digit
    else if (isdigit(buf[i])) {
      size_t toksize = 0;
      char isFloat = 0;
      while (isdigit(buf[i]) || buf[i] == '.') {
        if (buf[i] == '.')
          isFloat = 1;
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
      fprintf(stderr, "Unrecognized token %c.\n", buf[i]);
      dyn_destroy(toks);
      exit(1);
    }
  }
}
