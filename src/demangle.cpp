/******************************************************************************\
 *           ___        __                                                    *
 *          /\_ \    __/\ \                                                   *
 *          \//\ \  /\_\ \ \____    ___   _____   _____      __               *
 *            \ \ \ \/\ \ \ '__`\  /'___\/\ '__`\/\ '__`\  /'__`\             *
 *             \_\ \_\ \ \ \ \L\ \/\ \__/\ \ \L\ \ \ \L\ \/\ \L\.\_           *
 *             /\____\\ \_\ \_,__/\ \____\\ \ ,__/\ \ ,__/\ \__/.\_\          *
 *             \/____/ \/_/\/___/  \/____/ \ \ \/  \ \ \/  \/__/\/_/          *
 *                                          \ \_\   \ \_\                     *
 *                                           \/_/    \/_/                     *
 *                                                                            *
 * Copyright (C) 2011-2013                                                    *
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * This file is part of libcppa.                                              *
 * libcppa is free software: you can redistribute it and/or modify it under   *
 * the terms of the GNU Lesser General Public License as published by the     *
 * Free Software Foundation; either version 2.1 of the License,               *
 * or (at your option) any later version.                                     *
 *                                                                            *
 * libcppa is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with libcppa. If not, see <http://www.gnu.org/licenses/>.            *
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
