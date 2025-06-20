# minic

A basic compiler for the C99 programming language written in C.
To compile and run the compiler on a short test program ([simpletest.c](simpletest.c)), run `make`.

## Dynamic Arrays

Support for generic dynamic arrays has been added with the addition of [src/utils/dynarray.h](src/utils/dynarray.h). Testing for this module has been included in the test directory and can be run using `make test`.

Currently, the resize factor for my implementation of this structure is 1.5[^1], although I have seen performance improvement for resize factors closer to 2.[^2]

## Tokens

All lexemes for the C99 programming language are supported. This includes:

  - Keywords (see <https://en.cppreference.com/w/c/keyword/>)
  - Operators (+, -, *, /, =, bitwise operators, comparators, etc.)
  - Literals (integer, float, character, and string)
  - Special characters for preprocessor directives (\\, #)

## Parsing

Currently, parsing support has been added for the grammar defined in [grammar.bnf](grammar.bnf).[^3] This includes the following kinds of statements:

  - Function definitions
  - Variable declarations with assignment
  - Arbitrary-length and -depth expressions
  - Type checking and conversion between floating points and integers
  - Conditional statements (if, else-if, and else)
  - While loops

For the foreseeable future, the plan is to continue slowly rolling out features of the C language until most of the language's functionality is implemented. Obviously, this will take many months.

## Semantic Analysis

The semantic analyzer is simplistic, but it performs proper type conversion for expressions and identifiers between float and integer types. This can be observed in the LLVM IR output by the `trunc`, `fptrunc`, `sext`, `fpext`, `sitofp`, and `fptosi` instructions.

Some semantic analysis is delegated to the parser, which checks for variable and function declaration when they are used.

## Performance

While the program is not unbearably slow for small C programs, the performance of this program is not fully optimized (nor is the code's conciseness). Performance can be accelerated using `make release` which enables the `-O3` flag during compilation. 

To test the efficiency of memory usage (for both debug and release versions), `leaks` was used to assess the footprint of the binary when lexing the [test/chunkmesh.c](test/chunkmesh.c) file. The unoptimized binary left a physical footprint of 1752KB, while the most opitimized binary left a physical footprint of 1712KB. The program itself only allocates 14KB of memory using `malloc()` calls.

Memory allocation strategies differ depending on their context (which should not be a profound statement). For example, the standard library heap allocator is used for resizing the dynamic array structure, while an arena allocator is used for the abstract syntax tree because it better reflects the use case and improves performance.

[^1]: Or rather, `newSize = ceil(1.5 * oldSize)`.
[^2]: See <https://www.youtube.com/watch?v=GZPqDvG615k> for further exploration of this topic.
[^3]: It should be noted that code generation has not yet been implemented for all statement types, however (e.g., while loops).
