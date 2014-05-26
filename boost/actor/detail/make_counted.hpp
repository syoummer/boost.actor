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


#ifndef BOOST_ACTOR_DETAIL_MAKE_COUNTED_HPP
#define BOOST_ACTOR_DETAIL_MAKE_COUNTED_HPP

#include "boost/intrusive_ptr.hpp"

#include "boost/actor/ref_counted.hpp"

#include "boost/actor/mixin/memory_cached.hpp"

#include "boost/actor/detail/memory.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename T, typename... Ts>
typename std::enable_if<
    mixin::is_memory_cached<T>::value,
    intrusive_ptr<T>
>::type
make_counted(Ts&&... args) {
    return {detail::memory::create<T>(std::forward<Ts>(args)...)};
}

template<typename T, typename... Ts>
typename std::enable_if<
    not mixin::is_memory_cached<T>::value,
    intrusive_ptr<T>
>::type
make_counted(Ts&&... args) {
    return {new T(std::forward<Ts>(args)...)};
}

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_MAKE_COUNTED_HPP
