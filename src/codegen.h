#pragma once

#include "parser.h"
#include "tokens.h"
#include "utils/ast.h"
#include "utils/llvm.h"
#include <fcntl.h>
#include <stdio.h>

void generate_x64(ast_node *root, FILE *out);
void generate_arm(ast_node *root, FILE *out);
void generate_llvm(ast_node *root, FILE *out);
