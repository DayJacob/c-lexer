#include "llvm.h"

const char *asLLVMType(TokenType type) {
  switch (type) {
    case CHAR:   return "i8";
    case SHORT:  return "i16";
    case INT:    return "i32";
    case LONG:   return "i64";
    case VOID:   return "void";
    case DOUBLE: return "double";
    case FLOAT:  return "float";
    default:     return "";
  }
}

TokenType asBasicType(TokenType type) {
  switch (type) {
    case CHAR:
    case SHORT:
    case INT:
    case LONG:   return INT;
    case DOUBLE:
    case FLOAT:  return FLOAT;
    default:     return EMPTY;
  }
}

TokenType getStrongerType(TokenType left, TokenType right) {
  if (left == right)
    return left;

  if (getAlignment(left) > getAlignment(right))
    return left;
  else if (getAlignment(left) < getAlignment(right))
    return right;
  else {
    switch (left) {
      case FLOAT:
      case INT:    return FLOAT;
      case DOUBLE:
      case LONG:   return DOUBLE;
      default:     return DOUBLE;
    }
  }
}

const size_t getAlignment(TokenType type) {
  switch (type) {
    case CHAR:   return 1;
    case SHORT:  return 2;
    case INT:
    case FLOAT:  return 4;
    case LONG:
    case DOUBLE: return 8;
    default:     return 0;
  }
}
