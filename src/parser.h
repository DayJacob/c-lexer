#pragma once

#include "parser.h"
#include "tokens.h"
#include "utils/arena.h"
#include "utils/ast.h"
#include "utils/dynarray.h"
#include <stdbool.h>

double parseNum(str, size_t);
str parseString(str, size_t);

bool isType(TokenType);
bool isNumberLiteral(TokenType);

ast_node *try_parse_param(str, arena_t, dyn_array *, size_t *);
ast_node *try_parse_factor(str, arena_t, dyn_array *, size_t *);
ast_node *try_parse_term(str, arena_t, dyn_array *, size_t *);
ast_node *try_parse_cond(str, arena_t, dyn_array *, size_t *);
ast_node *try_parse_expr(str, arena_t, dyn_array *, size_t *);
ast_node *try_parse_stmt(str, arena_t, dyn_array *, size_t *);
ast_node *try_parse_scope(char *, str, arena_t, dyn_array *, size_t *);
ast_node *try_parse_funcdecl(str, arena_t, dyn_array *, size_t *);
ast_node *try_parse_prgm(str, arena_t, dyn_array *);

void parse(str buf, dyn_array *toks, ast_node **ast);
