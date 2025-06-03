#pragma once

#include "parser.h"
#include "tokens.h"
#include "utils/ast.h"

void parse(str buf, dyn_array *toks, ast_node **ast);
