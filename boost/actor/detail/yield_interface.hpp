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


#ifndef BOOST_ACTOR_YIELD_INTERFACE_HPP
#define BOOST_ACTOR_YIELD_INTERFACE_HPP

#include <string>

namespace boost {
namespace actor {
namespace detail {

struct cs_thread;

enum class yield_state : int {
    // yield() wasn't called yet
    invalid,
    // actor is still ready
    ready,
    // actor waits for messages
    blocked,
    // actor finished execution
    done
};

// return to the scheduler / worker
void yield(yield_state);

// switches to @p what and returns to @p from after yield(...)
yield_state call(detail::cs_thread* what, detail::cs_thread* from);

} // namespace detail
} // namespace actor
} // namespace boost

namespace boost {
namespace actor {

std::string to_string(detail::yield_state ys);

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_YIELD_INTERFACE_HPP
