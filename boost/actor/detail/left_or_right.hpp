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


#ifndef BOOST_ACTOR_LEFT_OR_RIGHT_HPP
#define BOOST_ACTOR_LEFT_OR_RIGHT_HPP

#include "boost/actor/unit.hpp"

namespace boost {
namespace actor {
namespace detail {

/**
 * @brief Evaluates to @p Right if @p Left == unit_t, @p Left otherwise.
 */
template<typename Left, typename Right>
struct left_or_right {
    typedef Left type;
};

template<typename Right>
struct left_or_right<unit_t, Right> {
    typedef Right type;
};

template<typename Right>
struct left_or_right<unit_t&, Right> {
    typedef Right type;
};

template<typename Right>
struct left_or_right<const unit_t&, Right> {
    typedef Right type;
};

/**
 * @brief Evaluates to @p Right if @p Left != unit_t, @p unit_t otherwise.
 */
template<typename Left, typename Right>
struct if_not_left {
    typedef unit_t type;
};

template<typename Right>
struct if_not_left<unit_t, Right> {
    typedef Right type;
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_LEFT_OR_RIGHT_HPP
