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


#ifndef BOOST_ACTOR_TUPLE_ITERATOR_HPP
#define BOOST_ACTOR_TUPLE_ITERATOR_HPP

#include <cstddef>

#include "boost/actor/config.hpp"

namespace boost {
namespace actor {
namespace detail {

template<class Tuple>
class message_iterator {

    size_t m_pos;
    const Tuple* m_tuple;

 public:

    inline message_iterator(const Tuple* tup, size_t pos = 0)
        : m_pos(pos), m_tuple(tup) {
    }

    message_iterator(const message_iterator&) = default;

    message_iterator& operator=(const message_iterator&) = default;

    inline bool operator==(const message_iterator& other) const {
        BOOST_ACTOR_REQUIRE(other.m_tuple == other.m_tuple);
        return other.m_pos == m_pos;
    }

    inline bool operator!=(const message_iterator& other) const {
        return !(*this == other);
    }

    inline message_iterator& operator++() {
        ++m_pos;
        return *this;
    }

    inline message_iterator& operator--() {
        BOOST_ACTOR_REQUIRE(m_pos > 0);
        --m_pos;
        return *this;
    }

    inline message_iterator operator+(size_t offset) {
        return {m_tuple, m_pos + offset};
    }

    inline message_iterator& operator+=(size_t offset) {
        m_pos += offset;
        return *this;
    }

    inline message_iterator operator-(size_t offset) {
        BOOST_ACTOR_REQUIRE(m_pos >= offset);
        return {m_tuple, m_pos - offset};
    }

    inline message_iterator& operator-=(size_t offset) {
        BOOST_ACTOR_REQUIRE(m_pos >= offset);
        m_pos -= offset;
        return *this;
    }

    inline size_t position() const { return m_pos; }

    inline const void* value() const {
        return m_tuple->at(m_pos);
    }

    inline const uniform_type_info* type() const {
        return m_tuple->type_at(m_pos);
    }

    inline message_iterator& operator*() { return *this; }

};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_TUPLE_ITERATOR_HPP
