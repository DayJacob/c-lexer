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

ast_node *try_parse_param(str, dyn_array *, size_t *);
ast_node *try_parse_factor(str, dyn_array *, size_t *);
ast_node *try_parse_term(str, dyn_array *, size_t *);
ast_node *try_parse_cond(str, dyn_array *, size_t *);
ast_node *try_parse_expr(str, dyn_array *, size_t *);
ast_node *try_parse_stmt(str, dyn_array *, size_t *);
ast_node *try_parse_scope(str, dyn_array *, size_t *);
ast_node *try_parse_funcdecl(str, dyn_array *, size_t *);
ast_node *try_parse_prgm(str, dyn_array *);

void parse(str buf, dyn_array *toks, ast_node **ast);
