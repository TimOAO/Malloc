#include <string>
#include <Assert.hpp>

AssertionFailure::AssertionFailure(const std::string& msg): m_error(msg) {
}

const std::string& AssertionFailure::what() const {
    return m_error;
}