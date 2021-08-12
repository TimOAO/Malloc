#If you want to use clang or some other compiler, change this. Must be in your $PATH
CC=g++ # Compiler

# App's source files are src/*.cpp
SRCS=$(wildcard src/*.cpp)

# App's include directory is include
INCLUDE=include

# Set of App's header files is include/*.hpp
HEADERS=$(wildcard $(INCLUDE)/*.hpp)

# Tests' include directory is test/include
TEST_INCLUDE=test/include

# Tests' header files are test/include/*.hpp
TEST_HEADERS=$(wildcard $(TEST_INCLUDE)/*.hpp)
TEST_SRCS=$(wildcard test/src/*.cpp)

# Compute .o files for app from src
OBJ=$(addsuffix .o, $(basename $(SRCS)))

# Compute .o fiels for tests from test srcs
TEST_OBJ=$(addsuffix .o, $(basename $(TEST_SRCS)))

# The name of your executable. You'll probably want to put this in your
# .gitignore so you don't accidently check it in.
BIN=example

# The name of your test executable. You'll probably want to also put this
# in .gitignore so you don't accidently check it in.
TEST_BIN=tests

# Compiler flags passed to CC when producting .o files
CPPFLAGS=-std=c++17 -g

# Default target that builds your executable; builds, and runs its tests.
all: $(BIN) test

# rule to run tests. Depends on building the tests.
test: $(TEST_BIN)
	./$(TEST_BIN)


# These rules compile your executable's cpp files into .o files.
# Changing a cpp file results in the minimal stuff rebuilding.
# Changing a header rebuilds everything.
# cpp files for your executable may only #include system headers, executable headers (include/),
# and *cannot* include test headers (test/include/). Executables should not depend on their
# tests.
Main.o: Main.cpp $(HEADERS)
	$(CC) -I$(INCLUDE) $(CPPFLAGS) -c -o $@ $<

src/%.o: src/%.cpp $(HEADERS)
	$(CC) -I$(INCLUDE) $(CPPFLAGS) -c -o $@ $<

# These rules compile your tests' cpp files into .o files.
# Chaging a cpp file in either your executable or tests results in minimal rebuild.
# Changing a header in executable or tests rebuids everything.
# cpp files for your tests may #include system headers, executable headers (include/),
# and test headers (test/include/)
TestMain.o: TestMain.cpp $(HEADERS) $(TEST_HEADERS)
	$(CC) -I$(INCLUDE) -I$(TEST_INCLUDE) $(CPPFLAGS) -c -o $@ $<

test/src/%.o: test/src/%.cpp $(HEADERS) $(TEST_HEADERS)
	$(CC) -I$(INCLUDE) -I$(TEST_INCLUDE) $(CPPFLAGS) -c -o $@ $<

# Link your executable
$(BIN): $(OBJ) Main.o
	$(CC) -o $(BIN) $(OBJ) Main.o -lpthread

# Link the test exectuable
$(TEST_BIN): $(OBJ) $(TEST_OBJ) $(HEADERS) $(TEST_HEADERS) TestMain.o
	$(CC) -o $(TEST_BIN) $(OBJ) $(TEST_OBJ) TestMain.o -lpthread

# Delete everything.
clean:
	-rm $(OBJ)
	-rm $(BIN)
	-rm $(TEST_OBJ)
	-rm $(TEST_BIN)
	-rm Main.o
	-rm TestMain.o