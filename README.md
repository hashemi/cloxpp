# cloxpp

This project follows Bob Nystrom's excellent book, [Crafting Interpreters](http://www.craftinginterpreters.com) which takes you through the process of writing an interpreter for a language called Lox.

The book goes over two implementations:

1. A tree-walking interpreter written in Java. I have ported that interpreter to Swift in [slox](https://github.com/hashemi/slox).

2. A bytecode interpreter written in C. I'm porting that to Swift ([bslox](https://github.com/hashemi/bslox)) and C++ (this repo).

The book is being released as chapters are completed, one chapter at a time.

## Progress
This port implements code from the following chapters of Part III of the book:

14. Chunks of Bytecode.
15. A Virtual Machine.
16. Scanning on Demand.
17. Compiling Expressions.
18. Types of Values.
19. Strings.
20. Hash Tables. (no code, using `std` containers)
21. Globals Variables.
22. Local Variables.
23. Jumping Back and Forth.
24. Calls and Functions.
25. Closures.

## Tests

The test suite is from the reference C implementation. To run the tests:

```
./test_cloxpp.py chap25_closures
```

The test script assumes that the binary is in `build/Release/cloxpp`, which is where it ends up after running `xcodebuild` from the command line.

## Goals & Design
My goal in this project is to become more proficient in C++.

## License
MIT