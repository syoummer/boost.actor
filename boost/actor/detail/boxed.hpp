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


#ifndef BOOST_ACTOR_BOXED_HPP
#define BOOST_ACTOR_BOXED_HPP

#include "boost/actor/anything.hpp"
#include "boost/actor/detail/wrapped.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename T>
struct boxed {
    typedef detail::wrapped<T> type;
};

template<typename T>
struct boxed< detail::wrapped<T> > {
    typedef detail::wrapped<T> type;
};

template<>
struct boxed<anything> {
    typedef anything type;
};

template<typename T>
struct is_boxed {
    static constexpr bool value = false;
};

template<typename T>
struct is_boxed< detail::wrapped<T> > {
    static constexpr bool value = true;
};

template<typename T>
struct is_boxed<detail::wrapped<T>()> {
    static constexpr bool value = true;
};

template<typename T>
struct is_boxed<detail::wrapped<T>(&)()> {
    static constexpr bool value = true;
};

template<typename T>
struct is_boxed<detail::wrapped<T>(*)()> {
    static constexpr bool value = true;
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_BOXED_HPP
