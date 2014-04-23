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


#ifndef BOOST_ACTOR_COMPARE_TUPLES_HPP
#define BOOST_ACTOR_COMPARE_TUPLES_HPP

#include "boost/actor/get.hpp"

#include "boost/actor/util/type_list.hpp"
#include "boost/actor/util/type_traits.hpp"

#include "boost/actor/detail/safe_equal.hpp"

namespace boost {
namespace actor {
namespace detail {

template<size_t N, template<typename...> class Tuple, typename... Ts>
const typename util::type_at<N, Ts...>::type&
do_get(const Tuple<Ts...>& t) {
    return ::boost::actor::get<N, Ts...>(t);
}

template<size_t N, typename LhsTuple, typename RhsTuple>
struct cmp_helper {
    inline static bool cmp(const LhsTuple& lhs, const RhsTuple& rhs) {
        return    detail::safe_equal(do_get<N>(lhs), do_get<N>(rhs))
               && cmp_helper<N-1, LhsTuple, RhsTuple>::cmp(lhs, rhs);
    }
};

template<typename LhsTuple, typename RhsTuple>
struct cmp_helper<0, LhsTuple, RhsTuple> {
    inline static bool cmp(const LhsTuple& lhs, const RhsTuple& rhs) {
        return do_get<0>(lhs) == do_get<0>(rhs);
    }
};

} // namespace detail
} // namespace actor
} // namespace boost

namespace boost {
namespace actor {
namespace util {

template<template<typename...> class LhsTuple, typename... LhsTs,
         template<typename...> class RhsTuple, typename... RhsTs>
bool compare_tuples(const LhsTuple<LhsTs...>& lhs,
                    const RhsTuple<RhsTs...>& rhs) {
    static_assert(sizeof...(LhsTs) == sizeof...(RhsTs),
                  "could not compare tuples of different size");

    static_assert(tl_binary_forall<
                      type_list<LhsTs...>,
                      type_list<RhsTs...>,
                      is_comparable
                  >::value,
                  "types of lhs are not comparable to the types of rhs");

    return detail::cmp_helper<(sizeof...(LhsTs) - 1),
                              LhsTuple<LhsTs...>,
                              RhsTuple<RhsTs...>>::cmp(lhs, rhs);
}

template<template<typename...> class LhsTuple, typename... LhsTs,
         template<typename...> class RhsTuple, typename... RhsTs>
bool compare_first_elements(const LhsTuple<LhsTs...>& lhs,
                            const RhsTuple<RhsTs...>& rhs) {
    typedef typename tl_zip<
                util::type_list<LhsTs...>,
                util::type_list<RhsTs...>
            >::type
            zipped_types;

    static_assert(tl_zipped_forall<zipped_types, is_comparable>::value,
                  "types of lhs are not comparable to the types of rhs");

    return detail::cmp_helper<(zipped_types::size - 1),
                              LhsTuple<LhsTs...>,
                              RhsTuple<RhsTs...>>::cmp(lhs, rhs);
}

} // namespace util
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_COMPARE_TUPLES_HPP
