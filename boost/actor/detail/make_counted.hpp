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


#ifndef BOOST_ACTOR_DETAIL_MAKE_COUNTED_HPP
#define BOOST_ACTOR_DETAIL_MAKE_COUNTED_HPP

#include "boost/intrusive_ptr.hpp"

#include "boost/actor/ref_counted.hpp"
#include "boost/actor/memory_cached.hpp"

#include "boost/actor/detail/memory.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename T, typename... Ts>
typename std::enable_if<is_memory_cached<T>::value, intrusive_ptr<T>>::type
make_counted(Ts&&... args) {
    return {detail::memory::create<T>(std::forward<Ts>(args)...)};
}

template<typename T, typename... Ts>
typename std::enable_if<not is_memory_cached<T>::value, intrusive_ptr<T>>::type
make_counted(Ts&&... args) {
    return {new T(std::forward<Ts>(args)...)};
}

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_MAKE_COUNTED_HPP
