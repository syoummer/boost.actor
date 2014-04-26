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


#ifndef BOOST_ACTOR_VALUE_GUARD_HPP
#define BOOST_ACTOR_VALUE_GUARD_HPP

#include <type_traits>

#include "boost/actor/unit.hpp"

#include "boost/actor/detail/type_list.hpp"
#include "boost/actor/detail/type_traits.hpp"
#include "boost/actor/detail/arg_match_t.hpp"

#include "boost/actor/detail/boxed.hpp"
#include "boost/actor/detail/safe_equal.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename T>
struct vg_cmp {
    template<typename U>
    inline static bool _(const T& lhs, const U& rhs) {
        return detail::safe_equal(lhs, rhs);
    }
    template<typename U>
    inline static bool _(const T& lhs, const std::reference_wrapper<U>& rhs) {
        return detail::safe_equal(lhs, rhs.get());
    }
};

template<>
struct vg_cmp<unit_t> {
    template<typename T>
    inline static bool _(const unit_t&, const T&) {
        return true;
    }
};

template<typename T>
struct is_match_helper {
    static constexpr bool value =    detail::is_callable<T>::value
                                  || detail::is_boxed<T>::value;
};

template<>
struct is_match_helper<anything> : std::true_type { };

template<>
struct is_match_helper<detail::arg_match_t> : std::true_type { };

struct filtered_construct_tuple_recursion_end { };

// filters callables and instances of `anything` before initializing the tuple
template<typename Tup, typename U, typename... Us>
typename std::enable_if<not is_match_helper<U>::value>::type
filtered_construct_tuple(Tup& tup, const U& v, const Us&... vs) {
    // push first element back (beyond filtered_construct_tuple_recursion_end_t)
    filtered_construct_tuple(tup, vs..., v);
}

template<typename Tup, typename... Us>
void filtered_construct_tuple(Tup& tup,
                              const filtered_construct_tuple_recursion_end&,
                              const Us&... vs) {
    // end of recursion, init tuple now
    tup = std::forward_as_tuple(vs...);
}

inline void filtered_construct_tuple(std::tuple<>&,
                              const filtered_construct_tuple_recursion_end&) {
    // well, there's not really something to do here ...
}

template<typename Tup, typename U, typename... Us>
typename std::enable_if<is_match_helper<U>::value>::type
filtered_construct_tuple(Tup& tup,
                         const U&,
                         const Us&... vs) {
    // filter first argument
    filtered_construct_tuple(tup, vs...);
}

struct value_guard_zipper {
    typedef bool result_type;
    inline bool operator()() const {
        // end of recursion
        return true;
    }
    template<typename T, typename U, typename... Vs>
    inline bool operator()(const std::tuple<T, U>& fwd, const Vs&... args) const {
        return    vg_cmp<T>::_(std::get<0>(fwd), std::get<1>(fwd))
               && (*this)(args...);
    }
};

template<typename FilteredPattern>
class value_guard {

 public:

    value_guard() = default;
    value_guard(const value_guard&) = default;

    template<typename... Ts>
    value_guard(const Ts&... args) {//: m_args(vg_fwd(args)...) { }
        filtered_construct_tuple(m_args, args..., filtered_construct_tuple_recursion_end{});
    }

    template<typename... Ts>
    inline bool operator()(const Ts&... args) const {
        value_guard_zipper vgz;
        auto targs = std::forward_as_tuple(args...);
        return detail::tuple_zip(vgz, detail::get_indices(m_args), m_args, targs);
    }

 private:

    typename detail::tl_apply<FilteredPattern, std::tuple>::type m_args;

};

typedef value_guard<detail::empty_type_list> empty_value_guard;

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_VALUE_GUARD_HPP
