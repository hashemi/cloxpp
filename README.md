# cloxpp

This project follows Bob Nystrom's excellent book, [Crafting Interpreters](http://www.craftinginterpreters.com) which takes you through the process of writing an interpreter for a language called Lox.

The book goes over two implementations:

1. A tree-walking interpreter written in Java. I have ported that interpreter to Swift in [slox](https://github.com/hashemi/slox).

2. A bytecode interpreter written in C. I'm porting that to Swift ([bslox](https://github.com/hashemi/bslox)) and C++ (this repo).

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
26. Garbage Collection.
27. Classes and Instances.
28. Methods and Initializers.

## Tests

The test suite is from the reference C implementation. To run the tests:

```zsh
dart tool/bin/test.dart chap28_methods --interpreter build/Release/cloxpp
```

The command specifies `build/Release/cloxpp` as the binary, which is where it ends up after running `xcodebuild` from the command line.

For the test suite to run, you need to have the Dart programming language SDK installed. After that, you need to get the test runners dependencies by going to the `tool` directory and running:

```zsh
pub get
```

## Goals & Design
My goal in this project is to become more proficient in C++.

## License
MIT