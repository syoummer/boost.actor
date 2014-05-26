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


#ifndef BOOST_ACTOR_UNBOXED_HPP
#define BOOST_ACTOR_UNBOXED_HPP

#include <memory>

#include "boost/actor/detail/wrapped.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename T>
struct unboxed {
    typedef T type;
};

template<typename T>
struct unboxed< detail::wrapped<T> > {
    typedef typename detail::wrapped<T>::type type;
};

template<typename T>
struct unboxed<detail::wrapped<T> (&)()> {
    typedef typename detail::wrapped<T>::type type;
};

template<typename T>
struct unboxed<detail::wrapped<T> ()> {
    typedef typename detail::wrapped<T>::type type;
};

template<typename T>
struct unboxed<detail::wrapped<T> (*)()> {
    typedef typename detail::wrapped<T>::type type;
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_UNBOXED_HPP
