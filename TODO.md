# TODOS

- Refactor BNF grammar into EBNF ✅
- Add parsing support for if/else and scopes ✅
- Start basic code generation (LLVM IR, x86, and ARM) (LLVM ✅)
- Add parsing support for while and do-while ✅
- Refactor symbol table to be constructed in parsing phase, not codegen phase ✅
- Consider adding intermediate and implicit casting nodes into the AST (unary ops) ✅
- Consider adding CFG pass to optimize conditional statements
- Add type checking and conversion for generic statements (not just return statements)
- Consider refactoring AST memory allocation strategy to arena allocator instead of heap allocator ✅
- Document code with well-stated comments
- Profit

# For later
## parser.c
- Consider refactoring parsing functions to consume tokens less/peek tokens more
- Consider refactoring parser to better handle errors (result<T, E> type?)
- Add parsing support for for
- Add parsing support for pointer types

## codegen.c
- Investigate why floating point constants cause so much trouble

## analysis.c
- Add sufficient semantic analysis

## testing
- Make sufficient test cases for libraries (dynarray, ast, etc.) and lexer/parser/codegen
