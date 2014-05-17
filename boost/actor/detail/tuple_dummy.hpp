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
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * Distributed under the Boost Software License, Version 1.0. See             *
 * accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt  *
\******************************************************************************/










#ifndef BOOST_ACTOR_DETAIL_TUPLE_DUMMY_HPP
#define TUPLE_DUMMY_HPP

#include "boost/actor/fwd.hpp"

#include "boost/actor/detail/type_list.hpp"

#include "boost/actor/detail/message_iterator.hpp"

namespace boost {
namespace actor {
namespace detail {

struct tuple_dummy {
    typedef detail::empty_type_list types;
    typedef message_iterator<tuple_dummy> const_iterator;
    inline size_t size() const {
        return 0;
    }
    inline void* mutable_at(size_t) {
        return nullptr;
    }
    inline const void* at(size_t) const {
        return nullptr;
    }
    inline const uniform_type_info* type_at(size_t) const {
        return nullptr;
    }
    inline const std::type_info* type_token() const {
        return &typeid(detail::empty_type_list);
    }
    inline bool dynamically_typed() const {
        return false;
    }
    inline const_iterator begin() const {
        return {this};
    }
    inline const_iterator end() const {
        return {this, 0};
    }
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // TUPLE_DUMMY_HPP
