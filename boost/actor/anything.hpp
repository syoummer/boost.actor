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


#ifndef BOOST_ACTOR_ANYTHING_HPP
#define BOOST_ACTOR_ANYTHING_HPP

#include <type_traits>

namespace boost {
namespace actor {

/**
 * @brief Acts as wildcard expression in patterns.
 */
struct anything { };

/**
 * @brief Compares two instances of {@link anything}.
 * @returns @p false
 * @relates anything
 */
inline bool operator==(const anything&, const anything&) { return true; }

/**
 * @brief Compares two instances of {@link anything}.
 * @returns @p false
 * @relates anything
 */
inline bool operator!=(const anything&, const anything&) { return false; }

/**
 * @brief Checks wheter @p T is {@link anything}.
 */
template<typename T>
struct is_anything {
    static constexpr bool value = std::is_same<T, anything>::value;
};


} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_ANYTHING_HPP
