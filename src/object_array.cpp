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


#include "boost/actor/detail/object_array.hpp"

namespace boost {
namespace actor {
namespace detail {

object_array::object_array() : super(true) { }

object_array::object_array(const object_array& other) : super(true) {
    m_elements.reserve(other.m_elements.size());
    for (auto& e : other.m_elements) m_elements.push_back(e->copy());
}

object_array::~object_array() { }

void object_array::push_back(uniform_value what) {
    BOOST_ACTOR_REQUIRE(   what != nullptr
                        && what->val != nullptr
                        && what->ti != nullptr);
    m_elements.push_back(std::move(what));
}

void* object_array::mutable_at(size_t pos) {
    return m_elements[pos]->val;
}

size_t object_array::size() const {
    return m_elements.size();
}

object_array* object_array::copy() const {
    return new object_array(*this);
}

const void* object_array::at(size_t pos) const {
    BOOST_ACTOR_REQUIRE(pos < size());
    return m_elements[pos]->val;
}

const uniform_type_info* object_array::type_at(size_t pos) const {
    BOOST_ACTOR_REQUIRE(pos < size());
    return m_elements[pos]->ti;
}

const std::string* object_array::tuple_type_names() const {
    return nullptr; // get_tuple_type_names(*this);
}

} } // namespace actor
} // namespace boost::detail
