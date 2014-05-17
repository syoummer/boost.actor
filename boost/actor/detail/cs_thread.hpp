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


#ifndef BOOST_ACTOR_FIBER_HPP
#define BOOST_ACTOR_FIBER_HPP

namespace boost {
namespace actor {
namespace detail {

struct cst_impl;

// A cooperatively scheduled thread implementation
struct cs_thread {

    // Queries whether libcppa was compiled without cs threads on this platform.
    static const bool is_disabled_feature;

    // Creates a new cs_thread storing the context of the calling thread.
    cs_thread();

    // Creates a cs_thread that executes @p func(arg1)
    cs_thread(void (*func)(void*), void* arg1);

    ~cs_thread();

    // Swaps the context from @p source to @p target.
    static void swap(cs_thread& source, cs_thread& target);

    // pimpl
    cst_impl* m_impl;

};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_FIBER_HPP
