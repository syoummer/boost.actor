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


#ifndef BOOST_ACTOR_DETAIL_TUPLE_ZIP_HPP
#define BOOST_ACTOR_DETAIL_TUPLE_ZIP_HPP

#include <tuple>

#include "boost/actor/detail/int_list.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename F, long... Is, class Tup0, class Tup1>
auto tuple_zip(F& f, detail::int_list<Is...>, Tup0&& tup0, Tup1&& tup1)
-> decltype(std::make_tuple(f(std::get<Is>(tup0),
                              std::get<Is>(tup1))...)) {
    return std::make_tuple(f(std::get<Is>(tup0),
                             std::get<Is>(tup1))...);
}

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_TUPLE_ZIP_HPP
