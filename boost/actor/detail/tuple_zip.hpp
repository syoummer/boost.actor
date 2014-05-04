/******************************************************************************\
 *           ___        __                                                    *
 *          /\_ \    __/\ \                                                   *
 *          \//\ \  /\_\ \ \____    ___   _____   _____      __               *
 *            \ \ \ \/\ \ \ '__`\  /'___\/\ '__`\/\ '__`\  /'__`\             *
 *             \_\ \_\ \ \ \ \L\ \/\ \__/\ \ \L\ \ \ \L\ \/\ \L\.\_           *
 *             /\____\\ \_\ \_,__/\ \____\\ \ ,__/\ \ ,__/\ \__/.\_\          *
 *             \/____/ \/_/\/___/  \/____/ \ \ \/  \ \ \/  \/__/\/_/          *
 *                                          \ \_\   \ \_\                     *
 *                                           \/_/    \/_/                     *
 *                                                                            *
 * Copyright (C) 2011-2013                                                    *
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * This file is part of libcppa.                                              *
 * libcppa is free software: you can redistribute it and/or modify it under   *
 * the terms of the GNU Lesser General Public License as published by the     *
 * Free Software Foundation; either version 2.1 of the License,               *
 * or (at your option) any later version.                                     *
 *                                                                            *
 * libcppa is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with libcppa. If not, see <http://www.gnu.org/licenses/>.            *
\******************************************************************************/


#ifndef BOOST_ACTOR_DETAIL_TUPLE_ZIP_HPP
#define BOOST_ACTOR_DETAIL_TUPLE_ZIP_HPP

#include <tuple>

#include "boost/actor/detail/int_list.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename F, long... Is, class Tup0, class Tup1>
auto tuple_zip(F& f, detail::int_list<Is...>, Tup0& tup0, Tup1& tup1)
-> decltype(std::forward_as_tuple(f(std::get<Is>(tup0), std::get<Is>(tup1))...)) {
    return std::forward_as_tuple(f(std::get<Is>(tup0), std::get<Is>(tup1))...);
}

template<typename F, long... Is, class Tup0, class Tup1, class Tup2>
auto tuple_zip(F& f, detail::int_list<Is...>, Tup0& tup0, Tup1& tup1, Tup2& tup2)
-> decltype(std::forward_as_tuple(f(std::get<Is>(tup0), std::get<Is>(tup1), std::get<Is>(tup2))...)) {
    return std::forward_as_tuple(f(std::get<Is>(tup0), std::get<Is>(tup1), std::get<Is>(tup2))...);
}

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_TUPLE_ZIP_HPP
