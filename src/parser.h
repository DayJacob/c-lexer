#pragma once

#include "parser.h"
#include "tokens.h"
#include "utils/ast.h"

void parse(dyn_array *toks, ast_node *ast);
