#pragma once

#include "../tokens.h"
#include "ast.h"

const char *asLLVMType(TokenType);
TokenType asBasicType(TokenType);
TokenType getStrongerType(TokenType, TokenType);
const size_t getAlignment(TokenType);
