# c-lexer

A basic lexer for the C99 programming language written in C.
To compile and run the lexer on a short test program ([test.c]), run ```make```.

## Dynamic Arrays

Support for generic dynamic arrays has been added with the addition of [src/utils/dyn_array.h]. Testing for this module has been included in the test directory and can be run using `make test`.

Currently, the resize factor for my implementation of this structure is 1.5[^1], although I have seen performance improvement for resize factors closer to 2.[^2]

## Tokens

All lexemes for the C99 programming language are supported. This includes:

  - Keywords (see <https://en.cppreference.com/w/c/keyword/>)
  - Operators (+, -, *, /, =, bitwise operators, comparators, etc.)
  - Literals (integer, float, character, and string)
  - Special characters for preprocessor directives (\\, #)

## Performance

While the program is not unbearably slow for small C programs, the performance of this program is not fully optimized (nor is the code's conciseness). Performance can be accelerated using `make release` which enables the `-O3` flag during compilation. 

To test the efficiency of memory usage (for both debug and release versions), `leaks` was used to assess the footprint of the binary when lexing the [test/chunkmesh.c] file. The unoptimized binary left a physical footprint of 1752KB, while the most opitimized binary left a physical footprint of 1712KB. The program itself only allocates 14KB of memory using `malloc()` calls.

[^1] Or rather, `newSize = ceil(1.5 * oldSize)`.
[^2] See <https://www.youtube.com/watch?v=GZPqDvG615k> for further exploration of this topic.
