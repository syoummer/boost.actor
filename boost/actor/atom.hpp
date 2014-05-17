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


#ifndef BOOST_ACTOR_ATOM_HPP
#define BOOST_ACTOR_ATOM_HPP

#include <string>

#include "boost/actor/detail/atom_val.hpp"

namespace boost {
namespace actor {

/**
 * @brief The value type of atoms.
 */
enum class atom_value : std::uint64_t {
    /** @cond PRIVATE */
    dirty_little_hack = 37337
    /** @endcond */
};

/**
 * @brief Returns @p what as a string representation.
 * @param what Compact representation of an atom.
 * @returns @p what as string.
 */
std::string to_string(const atom_value& what);

/**
 * @brief Creates an atom from given string literal.
 * @param str String constant representing an atom.
 * @returns A compact representation of @p str.
 */
template<size_t Size>
constexpr atom_value atom(char const (&str) [Size]) {
    // last character is the NULL terminator
    static_assert(Size <= 11, "only 10 characters are allowed");
    return static_cast<atom_value>(detail::atom_val(str, 0xF));
}

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_ATOM_HPP
