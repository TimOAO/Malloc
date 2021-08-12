#pragma once
#include <string>
#include <cstdint>
#include <sstream>

class AssertionFailure {
private:
    std::string m_error;

public:
    AssertionFailure() = delete;
    AssertionFailure(const std::string& msg);
    const std::string& what() const;
};

#define ASSERT_EQ(x, y)\
    assertEq(__FILE__, __LINE__, x, y);

#define ASSERT_TRUE(x)\
    assertTrue(__FILE__, __LINE__, x);


static std::string u32ToString(uint32_t x) {
    std::stringstream s;

    s << x;

    return s.str();
}

template <typename T, typename U> void assertEq(
    const char* filename,
    uint32_t line,
    T x,
    U y
) {
    if (x != y) {
        std::stringstream s;

        s   << filename
            << ":"
            << line
            << ":"
            << "Assertion failure: Values are not equal. Left: "
            << x
            << " Right: "
            << y;

        throw AssertionFailure(s.str());
    }
}

template <typename T> void assertTrue(
    const char* filename,
    uint32_t line,
    T x
) {
    if (!x) {
        throw AssertionFailure(
            std::string(filename)
            + std::string(":")
            + u32ToString(line)
            + std::string(":")
            + std::string("Assertion failure: Condition not true")
        );
    }
}
