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


#ifndef BOOST_ACTOR_TDATA_HPP
#define BOOST_ACTOR_TDATA_HPP

#include <tuple>
#include <utility>

#include "boost/optional.hpp"

#include "boost/actor/detail/int_list.hpp"
#include "boost/actor/detail/rebindable_reference.hpp"

#include "boost/actor/detail/tuple_zip.hpp"

namespace boost {
namespace actor {
namespace detail {

struct rebinder {
    typedef void result_type;
    inline void operator()() const {
        // end of recursion
    }
    template<typename T, typename U, typename... Vs>
    inline void operator()(std::tuple<T, U> fwd, Vs&&... args) const {
        std::get<0>(fwd) = std::get<1>(fwd);
        (*this)(std::forward<Vs>(args)...);
    }
    template<typename T, typename U, typename... Vs>
    inline void operator()(std::tuple<detail::rebindable_reference<T>&, U> fwd, Vs&&... args) const {
        std::get<0>(fwd).rebind(std::get<1>(fwd));
        (*this)(std::forward<Vs>(args)...);
    }
};

template<typename... Ts, typename... Us>
void rebind_tdata(std::tuple<Ts...>& lhs, const std::tuple<Us...>& rhs) {
    static_assert(sizeof...(Ts) == sizeof...(Us), "tuples of unequal size");
    rebinder f;
    tuple_zip(f, get_indices(lhs), lhs, rhs);
}

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_TDATA_HPP
