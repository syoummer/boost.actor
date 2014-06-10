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


#include "boost/actor/message.hpp"
#include "boost/actor/message_handler.hpp"

#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/decorated_tuple.hpp"

namespace boost {
namespace actor {

message::message(detail::message_data* ptr) : m_vals(ptr) { }

message::message(message&& other) : m_vals(std::move(other.m_vals)) { }

message::message(const data_ptr& vals) : m_vals(vals) { }

message& message::operator=(message&& other) {
    m_vals.swap(other.m_vals);
    return *this;
}

void message::reset() {
    m_vals.reset();
}

void* message::mutable_at(size_t p) {
    BOOST_ACTOR_REQUIRE(m_vals);
    return m_vals->mutable_at(p);
}

const void* message::at(size_t p) const {
    BOOST_ACTOR_REQUIRE(m_vals);
    return m_vals->at(p);
}

const uniform_type_info* message::type_at(size_t p) const {
    BOOST_ACTOR_REQUIRE(m_vals);
    return m_vals->type_at(p);
}

bool message::equals(const message& other) const {
    BOOST_ACTOR_REQUIRE(m_vals);
    return m_vals->equals(*other.vals());
}

message message::drop(size_t n) const {
    BOOST_ACTOR_REQUIRE(m_vals);
    if (n == 0) return *this;
    if (n >= size()) return message{};
    return message{detail::decorated_tuple::create(m_vals, n)};
}

message message::drop_right(size_t n) const {
    BOOST_ACTOR_REQUIRE(m_vals);
    if (n == 0) return *this;
    if (n >= size()) return message{};
    std::vector<size_t> mapping(size() - n);
    size_t i = 0;
    std::generate(mapping.begin(), mapping.end(), [&] { return i++; });
    return message{detail::decorated_tuple::create(m_vals, std::move(mapping))};
}

optional<message> message::apply(message_handler handler) {
    return handler(*this);
}

} // namespace actor
} // namespace boost
