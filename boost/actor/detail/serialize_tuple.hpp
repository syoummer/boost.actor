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


#ifndef BOOST_ACTOR_SERIALIZE_TUPLE_HPP
#define BOOST_ACTOR_SERIALIZE_TUPLE_HPP

#include <cstddef>

#include "boost/actor/uniform_type_info.hpp"
#include "boost/actor/detail/type_list.hpp"

namespace boost { namespace actor { class serializer; } }

namespace boost {
namespace actor {
namespace detail {

template<typename List, size_t Pos = 0>
struct serialize_tuple {
    template<typename T>
    inline static void _(serializer& s, const T* tup) {
        s << uniform_typeid<typename List::head>()->name()
          << *reinterpret_cast<const typename List::head*>(tup->at(Pos));
        serialize_tuple<typename List::tail, Pos + 1>::_(s, tup);
    }
};

template<size_t Pos>
struct serialize_tuple<detail::empty_type_list, Pos> {
    template<typename T>
    inline static void _(serializer&, const T*) { }
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_SERIALIZE_TUPLE_HPP
