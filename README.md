# c-lexer

A basic lexer for the C99 programming language written in C.

## Dynamic Arrays

Support for generic dynamic arrays has been added with the addition of src/libs/dyn_array.h. Testing for this module has been included in the test directory.
Currently, the resize factor for my implementation of this structure is 1.5 (or rather, the ceiling thereof), although I have seen performance improvement for resize factors closer to 2.

## Tokens

Currently, there is no support for character literals (such as sequences like 'n'). Additionally, lexemes for preprocessor directives are not supported yet.

## Performance

While the program is not unbearably slow for small C programs, the performance of this program is not fully optimized (nor is the code's conciseness).
