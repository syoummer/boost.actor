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


#ifndef MAY_HAVE_TIMEOUT_HPP
#define MAY_HAVE_TIMEOUT_HPP

namespace boost {
namespace actor {

template<typename F>
struct timeout_definition;

class behavior;

template<typename T>
struct may_have_timeout {
    static constexpr bool value = false;
};

template<>
struct may_have_timeout<behavior> {
    static constexpr bool value = true;
};

template<typename F>
struct may_have_timeout<timeout_definition<F>> {
    static constexpr bool value = true;
};

} // namespace actor
} // namespace boost

#endif // MAY_HAVE_TIMEOUT_HPP
