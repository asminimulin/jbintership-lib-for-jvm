#pragma once

#include "forward_definitions.hpp"

#include <iostream>
#include <sstream>
#include <thread>


namespace logs {
namespace detail {

class LogStream {
public:
    LogStream(const char* file, const char* line) {
        m_stream << file << ':' << line;
    }

    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    template<typename T>
    LogStream& operator<< (const T& arg) {
        m_stream << ' ' << arg;
        return *this;
    }

    ~LogStream() {
        std::cerr << std::this_thread::get_id() << " | " << m_stream.str() << std::endl;
    }

private:
    std::stringstream m_stream;
};

struct DummyStream {
    template<typename T>
    DummyStream& operator<< (const T&) {
        return *this;
    }
};

}
}

#ifdef DEBUG
#define log (logs::detail::LogStream(__FILE__, MACRO_STR(__LINE__)))
#else
#define log logs::detail::DummyStream()
#endif // DEBUG
