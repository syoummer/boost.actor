/******************************************************************************\
 *                                                                            *
 *           ____                  _        _        _                        *
 *          | __ )  ___   ___  ___| |_     / \   ___| |_ ___  _ __            *
 *          |  _ \ / _ \ / _ \/ __| __|   / _ \ / __| __/ _ \| '__|           *
 *          | |_) | (_) | (_) \__ \ |_ _ / ___ \ (__| || (_) | |              *
 *          |____/ \___/ \___/|___/\__(_)_/   \_\___|\__\___/|_|              *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * Distributed under the Boost Software License, Version 1.0. See             *
 * accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt  *
\******************************************************************************/


#include <string>
#include <cstdlib>
#include <stdexcept>

#include "boost/config.hpp"

#include "boost/actor/detail/demangle.hpp"

#if defined(BOOST_GCC) || defined(BOOST_CLANG)
#   include <cxxabi.h>
#   include <stdlib.h>
#endif

namespace boost {
namespace actor {
namespace detail {

namespace {

// filter unnecessary characters from undecorated cstr
std::string filter_whitespaces(const char* cstr, size_t size = 0) {
    std::string result;
    if (size > 0) result.reserve(size);
    char c = *cstr;
    while (c != '\0') {
        if (c == ' ') {
            char previous_c = result.empty() ? ' ' : *(result.rbegin());
            // get next non-space character
            for (c = *++cstr; c == ' '; c = *++cstr) { }
            if (c != '\0') {
                // skip whitespace unless it separates two alphanumeric
                // characters (such as in "unsigned int")
                if (isalnum(c) && isalnum(previous_c)) {
                    result += ' ';
                    result += c;
                }
                else {
                    result += c;
                }
                c = *++cstr;
            }
        }
        else {
            result += c;
            c = *++cstr;
        }
    }
    return result;
}

} // namespace <anonymous>

#if defined(BOOST_GCC) || defined(BOOST_CLANG)

std::string demangle(const char* decorated) {
    using std::string;
    size_t size;
    int status;
    std::unique_ptr<char, void (*)(void*)> undecorated{
        abi::__cxa_demangle(decorated, nullptr, &size, &status),
        std::free
    };
    if (status != 0) {
        string error_msg = "Could not demangle type name ";
        error_msg += decorated;
        throw std::logic_error(error_msg);
    }
    // the undecorated typeid name
    string result = filter_whitespaces(undecorated.get(), size);
#   ifdef BOOST_CLANG
    // replace "std::__1::" with "std::" (fixes strange clang names)
    string needle = "std::__1::";
    string fixed_string = "std::";
    for (auto pos = result.find(needle); pos != string::npos; pos = result.find(needle)) {
        result.replace(pos, needle.size(), fixed_string);
    }
#   endif
    return result;
}

#elif defined(BOOST_MSVC)

string demangle(const char* decorated) {
    // on MSVC, name() returns a human-readable version, all we need
    // to do is to remove "struct" and "class" qualifiers
    // (handled in to_uniform_name)
    return filter_whitespaces(decorated);
}

#else
#   error "compiler or platform not supported"
#endif // defined(BOOST_GCC) || defined(BOOST_CLANG)


std::string demangle(const std::type_info& tinf) {
    return demangle(tinf.name());
}

} // namespace detail
} // namespace actor
} // namespace boost
