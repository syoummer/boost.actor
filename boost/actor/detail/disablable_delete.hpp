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


#ifndef BOOST_ACTOR_DISABLABLE_DELETE_HPP
#define BOOST_ACTOR_DISABLABLE_DELETE_HPP

namespace boost {
namespace actor {
namespace detail {

class disablable_delete {

 public:

    constexpr disablable_delete() : m_enabled(true) { }

    inline void disable() {
        m_enabled = false;
    }

    inline void enable() {
        m_enabled = true;
    }

    template<typename T>
    inline void operator()(T* ptr) {
        if (m_enabled) delete ptr;
    }

 private:

    bool m_enabled;

};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DISABLABLE_DELETE_HPP
