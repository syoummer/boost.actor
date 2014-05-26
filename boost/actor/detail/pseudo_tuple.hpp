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


#ifndef BOOST_ACTOR_PSEUDO_TUPLE_HPP
#define BOOST_ACTOR_PSEUDO_TUPLE_HPP

#include <cstddef>

#include "boost/actor/detail/type_traits.hpp"

namespace boost {
namespace actor {
namespace detail {

// tuple-like access to an array of void pointers
template<typename... T>
struct pseudo_tuple {
    typedef void* pointer;
    typedef const void* const_pointer;

    pointer data[sizeof...(T) > 0 ? sizeof...(T) : 1];

    inline const_pointer at(size_t p) const {
        return data[p];
    }

    inline pointer mutable_at(size_t p) {
        return data[p];
    }

    inline pointer& operator[](size_t p) {
        return data[p];
    }
};

template<size_t N, typename... Ts>
const typename detail::type_at<N, Ts...>::type& get(const detail::pseudo_tuple<Ts...>& tv) {
    static_assert(N < sizeof...(Ts), "N >= tv.size()");
    return *reinterpret_cast<const typename detail::type_at<N, Ts...>::type*>(tv.at(N));
}

template<size_t N, typename... Ts>
typename detail::type_at<N, Ts...>::type& get(detail::pseudo_tuple<Ts...>& tv) {
    static_assert(N < sizeof...(Ts), "N >= tv.size()");
    return *reinterpret_cast<typename detail::type_at<N, Ts...>::type*>(tv.mutable_at(N));
}

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_PSEUDO_TUPLE_HPP
