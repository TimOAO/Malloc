# Intro
This repository serves as a skeleton C++ project with no dependencies. It provides a makefile that compiles your application and a test harness.

## Changing your application
The entry point is `Main.cpp` in the root of the enlistment, which calls `mainImpl` in `src/Main.cpp`. This piggy backing feature allows you to write tests against `mainImpl` without invoking a separate command each time.

## Writing tests
For an example of a test, please look in `test/src/FactorialTest.cpp`. This file implements `runFactorialTests` that gets called in `test/src/TestMain.cpp`. If you wish to add more test suites, copy `test/src/FactorialTest.cpp`, change the method name from `runFactorialTests`. You'll need to declare the method in an hpp file you add under `test/include`, import this header in `TestMain.cpp`, and call it.

Test methods in this setup return the number of failed test cases.

`test/include/Testsuite.hpp` includes a number of simple utilities for defining and running tests. The `TestSuite` class is a collection of tests. The general flow is

1. Write a bunch of test methods that return void and take no parameters.
1. These test methods should call the `ASSERT_EQ` and `ASSERT_TRUE` macros to assert things that should be true. These macros will result in an exception on assertion violation.
1. In your test suite run method (e.g. `runFactorialTests`), add each test method to the suite with the `TEST` macro.
1. One you're added all your tests, call `suite.run()`, which will iterate over the test methods and run them. When running test methods, the test suite will catch any assertion failures and fail the test. `suite.run()` will return the number of failed tests.
1. Call your test suite run method in `testMain` in `test/src/TestMain.cpp`.

## Building and running tests
To compile your program and run all the tests, run
```
make -j8
```

The Makefile balances simplicity with intelligence. It will incrementally compile only things that change whenever you change a cpp file. However, header changes will rebuild everything. You don't need to change the Makefile to add new headers or sources for either your application or tests.

Tests are allowed to `#include` anything under the application's `include` directory or the tests' include directory (`test/include`). Your product may only `#include` files under `include`.

## Prerequisites
The makefile assumes you have the `g++` and `make` installed and in your path. If you need to change the compiler, change the `CC` variable on line 1 in the Makefile.

Follow the below instructions depending on your OS to install these prereqs.

### Debian, Ubuntu, many others
```
sudo apt update
sudo apt install g++ make
```

### macOS
```
TBD, probably using brew.
```