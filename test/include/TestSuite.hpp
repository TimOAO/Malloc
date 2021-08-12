#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include <string>

class Test {
    std::string m_name;
    std::function<void()> m_test;

public:
    Test() = delete;
    Test(const std::string& name, const std::function<void()> m_test);

    const std::string& name() const;
    void run() const;
};

#define TEST(suite, fn)\
    suite.add(Test(#fn, fn));

class TestSuite {
private:
    std::vector<Test> m_tests;

public:
    uint64_t run();
    void add(const Test& test);
};
