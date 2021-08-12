#include <iostream>
#include <string>
#include <cstdint>

#include <Assert.hpp>
#include <TestSuite.hpp>

Test::Test(const std::string& name, const std::function<void()> test): m_name(name), m_test(test) { }

const std::string& Test::name() const { return m_name; }

void Test::run() const { m_test(); }

uint64_t TestSuite::run() {
    uint64_t failures = 0;

    for (auto test : m_tests) {
        try {
            test.run();

            std::cout << "Test " << test.name() << " passed." << std::endl;
        } catch (AssertionFailure& e) {
            std::cout << "Test " << test.name() << " failed:" << std::endl;
            std::cout << "  " << e.what() << std::endl;

            failures++;
        }
    }

    return failures;
}

void TestSuite::add(const Test& test) {
    m_tests.push_back(test);
}