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

## Goals & Design
My goal in this project is to become more proficient in C++.

## License
MIT