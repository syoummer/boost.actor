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


#ifndef BOOST_ACTOR_MIXED_HPP
#define BOOST_ACTOR_MIXED_HPP

namespace boost {
namespace actor {

namespace detail {

template<class D, class B, template<class, class> class... Ms>
struct extend_helper;

template<class D, class B>
struct extend_helper<D, B> { typedef B type; };

template<class D,
         class B,
         template<class, class> class M,
         template<class, class> class... Ms>
struct extend_helper<D, B, M, Ms...> : extend_helper<D, M<B, D>, Ms...> { };

} // namespace detail

/**
 * @brief Allows convenient definition of types using mixins.
 *        For example, "extend<ar, T>::with<ob, fo>" is an alias for
 *        "fo<ob<ar, T>, T>".
 *
 * Mixins always have two template parameters: base type and
 * derived type. This allows mixins to make use of the curiously recurring
 * template pattern (CRTP). However, if none of the used mixins use CRTP,
 * the second template argument can be ignored (it is then set to Base).
 */
template<class Base, class Derived = Base>
struct extend {
    /**
     * @brief Identifies the combined type.
     */
    template<template<class, class> class... Mixins>
    using with = typename detail::extend_helper<Derived, Base, Mixins...>::type;
};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_MIXED_HPP
