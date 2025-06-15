# TODOS

- Refactor BNF grammar into EBNF ✅
- Add parsing support for if/else and scopes ✅
- Start basic code generation (LLVM IR, x86, and ARM) (LLVM ✅)
- Add parsing support for while and do-while ✅
- Add parsing support for for
- Add parsing support for pointer types
- Refactor symbol table to be constructed in parsing phase, not codegen phase ✅
- Consider refactoring symbol table pushes to be in AST, not parser (better performance?)
- Consider adding intermediate and implicit casting nodes into the AST (unary ops)
- Add sufficient semantic analysis in codegen phase
- Consider refactoring parser to better handle errors (result<T, E> type?)
- Consider refactoring parsing functions to consume tokens less/peek tokens more
- Add type checking and conversion for generic statements (not just return statements)
- Make sufficient test cases for libraries (dynarray, ast, etc.) and lexer/parser/codegen
- Consider refactoring AST memory allocation strategy to arena allocator instead of heap allocator ✅
- Document code with well-stated comments
- Profit
