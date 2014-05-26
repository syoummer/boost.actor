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
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the Boost Software License, Version 1.0. See             *
 * accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt  *
\******************************************************************************/


#include "boost/actor/atom.hpp"

namespace boost {
namespace actor {

std::string to_string(const atom_value& what) {
    auto x = static_cast<std::uint64_t>(what);
    std::string result;
    result.reserve(11);
    // don't read characters before we found the leading 0xF
    // first four bits set?
    bool read_chars = ((x & 0xF000000000000000) >> 60) == 0xF;
    std::uint64_t mask = 0x0FC0000000000000;
    for (int bitshift = 54; bitshift >= 0; bitshift -= 6, mask >>= 6) {
        if (read_chars) {
            result += detail::decoding_table[(x & mask) >> bitshift];
        }
        else if (((x & mask) >> bitshift) == 0xF) {
            read_chars = true;
        }
    }
    return result;
}

} // namespace actor
} // namespace boost
