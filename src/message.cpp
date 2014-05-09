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


#include "boost/actor/message.hpp"
#include "boost/actor/singletons.hpp"

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
    using namespace std;
    if (n == 0) return *this;
    if (n >= size()) return message{};
    vector<size_t> mapping(size() - n);
    size_t i = 0;
    generate(mapping.begin(), mapping.end(), [&] { return i++; });
    return message{detail::decorated_tuple::create(m_vals, move(mapping))};
}

} // namespace actor
} // namespace boost
