#include <TestSuite.hpp>
#include <MallocTest.hpp>

int testMain(int argc, const char* argv[]) {
    int fail = 0;
    
    fail += runMallocTests();

    return fail;
}