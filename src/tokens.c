#include "tokens.h"
#include "utils/dynarray.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#define max(a, b) ((((a) > (b)) ? (a) : (b)))

// Returns the line number of a character in a string
size_t getLineNo(str buf, size_t len, size_t pos) {
  assert(pos <= len, "Index out of bounds.");

  size_t line = 1;
  for (int i = 0; i <= pos; ++i) {
    if (at(buf, i) == '\n')
      ++line;
  }

  return line;
}

// Tokenizes buf into an array of Tokens.
void tokenize(str buf, dyn_array *toks, size_t len) {
  size_t i = 0; // Current character index

  while (i < len) {
    // The at() function performs runtime bounds checking on the string input.
    // If the index is out of bounds, the program panics.
    if (at(buf, i) == '/' && at(buf, i + 1) == '/') {
      // Ignore comments
      while (at(buf, i++) != '\n')
        ;
    }

    // Ignore whitespace
    else if (isspace(at(buf, i))) {
      ++i;
    }

    // Tokenize string literal
    else if (at(buf, i) == '"') {
      ++i;

      size_t toksize = 1;
      while (at(buf, i++) != '"')
        ++toksize;

      // The token ptr is allocated in the tokenize() function, but must be
      // freed later using freeTokens().
      Token *ptr = (Token *)malloc(sizeof(Token));
      ptr->type = STRINGLIT;
      ptr->start = i - toksize;

      dyn_push(toks, ptr);
    }

    // Character literal
    else if (at(buf, i) == '\'') {
      ++i;

      // Currently accepts arbitrary length character literals, might change
      // later.
      size_t toksize = 1;
      while (at(buf, i++) != '\'')
        ++toksize;

      Token *ptr = (Token *)malloc(sizeof(Token));
      ptr->type = CHARLIT;
      ptr->start = i - toksize;

      dyn_push(toks, ptr);
    }

    // Check special character
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

        case '{': ptr->type = LBRACE; break;
        case '}': ptr->type = RBRACE; break;
        case '[': ptr->type = LBRACKET; break;
        case ']': ptr->type = RBRACKET; break;
        case '(': ptr->type = LPAREN; break;
        case ')': ptr->type = RPAREN; break;

        case '=': {
          if (at(buf, i + 1) == '=') {
            ptr->type = EQEQ;
            ++i;
          } else
            ptr->type = EQUALS;

        } break;

        case ';': ptr->type = SEMI; break;
        case ':': ptr->type = COLON; break;
        case ',': ptr->type = COMMA; break;

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

        case '~': ptr->type = TILDE; break;
        case '.': ptr->type = PERIOD; break;
        case '#': ptr->type = HASH; break;

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

        case '?':  ptr->type = QUESTION; break;

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

    // Identifier/keyword
    // First character must be alphabetic, but following characters may be
    // alphanumeric or _
    else if (isalpha(at(buf, i))) {
      size_t toksize = 0;
      while (isalnum(at(buf, i)) || at(buf, i) == '_') {
        ++i;
        ++toksize;
      }

      // dupl() calls malloc, so bufcmp must be freed at the end of its
      // lifetime.
      char *bufcmp = dupl(buf, i - toksize, toksize);

      Token *ptr = (Token *)malloc(sizeof(Token));

      // Strlen is safe here since we are using it on a string literal.
      if (!strncmp(bufcmp, "auto", max(toksize, strlen("auto"))))
        ptr->type = AUTO;

      else if (!strncmp(bufcmp, "break", max(toksize, strlen("break"))))
        ptr->type = BREAK;

      else if (!strncmp(bufcmp, "case", max(toksize, strlen("case"))))
        ptr->type = CASE;

      else if (!strncmp(bufcmp, "char", max(toksize, strlen("char"))))
        ptr->type = CHAR;

      else if (!strncmp(bufcmp, "const", max(toksize, strlen("const"))))
        ptr->type = CONST;

      else if (!strncmp(bufcmp, "continue", max(toksize, strlen("continue"))))
        ptr->type = CONTINUE;

      else if (!strncmp(bufcmp, "default", max(toksize, strlen("default"))))
        ptr->type = DEFAULT;

      else if (!strncmp(bufcmp, "do", max(toksize, strlen("do"))))
        ptr->type = DO;

      else if (!strncmp(bufcmp, "double", max(toksize, strlen("double"))))
        ptr->type = DOUBLE;

      else if (!strncmp(bufcmp, "else", max(toksize, strlen("else"))))
        ptr->type = ELSE;

      else if (!strncmp(bufcmp, "enum", max(toksize, strlen("enum"))))
        ptr->type = ENUM;

      else if (!strncmp(bufcmp, "extern", max(toksize, strlen("extern"))))
        ptr->type = EXTERN;

      else if (!strncmp(bufcmp, "float", max(toksize, strlen("float"))))
        ptr->type = FLOAT;

      else if (!strncmp(bufcmp, "for", max(toksize, strlen("for"))))
        ptr->type = FOR;

      else if (!strncmp(bufcmp, "goto", max(toksize, strlen("goto"))))
        ptr->type = GOTO;

      else if (!strncmp(bufcmp, "if", max(toksize, strlen("if"))))
        ptr->type = IF;

      else if (!strncmp(bufcmp, "inline", max(toksize, strlen("inline"))))
        ptr->type = INLINE;

      else if (!strncmp(bufcmp, "int", max(toksize, strlen("int"))))
        ptr->type = INT;

      else if (!strncmp(bufcmp, "long", max(toksize, strlen("long"))))
        ptr->type = LONG;

      else if (!strncmp(bufcmp, "register", max(toksize, strlen("register"))))
        ptr->type = REGISTER;

      else if (!strncmp(bufcmp, "restrict", max(toksize, strlen("restrict"))))
        ptr->type = RESTRICT;

      else if (!strncmp(bufcmp, "return", max(toksize, strlen("return"))))
        ptr->type = RETURN;

      else if (!strncmp(bufcmp, "short", max(toksize, strlen("short"))))
        ptr->type = SHORT;

      else if (!strncmp(bufcmp, "signed", max(toksize, strlen("signed"))))
        ptr->type = SIGNED;

      else if (!strncmp(bufcmp, "sizeof", max(toksize, strlen("sizeof"))))
        ptr->type = SIZEOF;

      else if (!strncmp(bufcmp, "static", max(toksize, strlen("static"))))
        ptr->type = STATIC;

      else if (!strncmp(bufcmp, "struct", max(toksize, strlen("struct"))))
        ptr->type = STRUCT;

      else if (!strncmp(bufcmp, "switch", max(toksize, strlen("switch"))))
        ptr->type = SWITCH;

      else if (!strncmp(bufcmp, "typedef", max(toksize, strlen("typedef"))))
        ptr->type = TYPEDEF;

      else if (!strncmp(bufcmp, "union", max(toksize, strlen("union"))))
        ptr->type = UNION;

      else if (!strncmp(bufcmp, "unsigned", max(toksize, strlen("unsigned"))))
        ptr->type = UNSIGNED;

      else if (!strncmp(bufcmp, "void", max(toksize, strlen("void"))))
        ptr->type = VOID;

      else if (!strncmp(bufcmp, "volatile", max(toksize, strlen("volatile"))))
        ptr->type = VOLATILE;

      else if (!strncmp(bufcmp, "while", max(toksize, strlen("while"))))
        ptr->type = WHILE;

      else
        ptr->type = IDENT;

      ptr->start = i - toksize;

      dyn_push(toks, ptr);

      free(bufcmp);
      bufcmp = NULL;
    }

    // Numeric literal
    else if (isdigit(at(buf, i))) {
      size_t toksize = 0;
      bool isFloat = false; // Distinguishes between floats and integers
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

    // Panic on unrecognized characters.
    else {
      fprintf(stderr, "Unrecognized token %c (line %lu).\n", at(buf, i),
              getLineNo(buf, len, i));
      dyn_destroy(toks);
      exit(1);
    }
  }

  // Print tokens to stdout
  dump(toks);
}

// Frees the memory used by the tokenizer's malloc() calls.
void freeTokens(dyn_array *toks) {
  for (size_t i = 0; i < toks->len; ++i) {
    // Bounds checking is unnecessary because the condition is guaranteed by the
    // for loop.
    Token *t = (Token *)toks->el[i];
    free(t);
    t = NULL;
  }
}
