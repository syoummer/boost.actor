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


#ifndef BOOST_ACTOR_UNIT_HPP
#define BOOST_ACTOR_UNIT_HPP

namespace boost {
namespace actor {

struct unit_t {
    constexpr unit_t() { }
    constexpr unit_t(const unit_t&) { }
    template<typename T> explicit constexpr unit_t(T&&) { }
};

static constexpr unit_t unit = unit_t{};

template<typename T>
struct lift_void { typedef  T type; };

template<>
struct lift_void<void> { typedef unit_t type; };

template<typename T>
struct unlift_void { typedef  T type; };

template<>
struct unlift_void<unit_t> { typedef void type; };

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_UNIT_HPP
