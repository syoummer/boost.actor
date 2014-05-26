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


#ifndef CALL_HPP
#define CALL_HPP

#include "boost/actor/detail/int_list.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename F, long... Is, class Tuple>
inline auto apply_args(F& f, detail::int_list<Is...>, Tuple&& tup)
-> decltype(f(get<Is>(tup)...)) {
    return f(get<Is>(tup)...);
}

template<typename F, class Tuple, typename... Ts>
inline auto apply_args_prefixed(F& f, detail::int_list<>, Tuple&, Ts&&... args)
-> decltype(f(std::forward<Ts>(args)...)) {
    return f(std::forward<Ts>(args)...);
}

template<typename F, long... Is, class Tuple, typename... Ts>
inline auto apply_args_prefixed(F& f, detail::int_list<Is...>, Tuple& tup, Ts&&... args)
-> decltype(f(std::forward<Ts>(args)..., get<Is>(tup)...)) {
    return f(std::forward<Ts>(args)..., get<Is>(tup)...);
}

template<typename F, long... Is, class Tuple, typename... Ts>
inline auto apply_args_suffxied(F& f, detail::int_list<Is...>, Tuple& tup, Ts&&... args)
-> decltype(f(get<Is>(tup)..., std::forward<Ts>(args)...)) {
    return f(get<Is>(tup)..., std::forward<Ts>(args)...);
}

} // namespace detail
} // namespace actor
} // namespace boost

#endif // CALL_HPP
