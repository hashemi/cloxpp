# cloxpp

This project follows Bob Nystrom's excellent book, [Crafting Interpreters](http://www.craftinginterpreters.com) which takes you through the process of writing an interpreter for a language called Lox. The interpreter is ported from the original C interprter to C++.

## Progress

The interpreter is now fully implemented in terms of functionality. It implements code from the following chapters of section III of the book:

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
26. Garbage Collection. (not implemented, using `shared_ptr` instead for ref counting)
27. Classes and Instances.
28. Methods and Initializers.
29. Superclasses.

Next I'll be working on refactoring the code and improving its performance.

## Tests

The test suite is from the reference C implementation. To run the tests:

```zsh
dart tool/bin/test.dart chap29_superclasses --interpreter build/Release/cloxpp
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