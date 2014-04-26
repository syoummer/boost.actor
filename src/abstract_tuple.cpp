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


#include "boost/actor/detail/message_data.hpp"

namespace boost {
namespace actor {
namespace detail {

message_data::message_data(bool is_dynamic) : m_is_dynamic(is_dynamic) { }

bool message_data::equals(const message_data &other) const {
    return    this == &other
           || (   size() == other.size()
               && std::equal(begin(), end(), other.begin(), detail::full_eq));
}

message_data::message_data(const message_data& other)
: m_is_dynamic(other.m_is_dynamic) { }

const std::type_info* message_data::type_token() const {
    return &typeid(void);
}

const void* message_data::native_data() const {
    return nullptr;
}

void* message_data::mutable_native_data() {
    return nullptr;
}

std::string get_tuple_type_names(const detail::message_data& tup) {
    std::string result = "@<>";
    for (size_t i = 0; i < tup.size(); ++i) {
        auto uti = tup.type_at(i);
        result += "+";
        result += uti->name();
    }
    return result;
}

message_data* message_data::ptr::get_detached() {
    auto p = m_ptr.get();
    if (!p->unique()) {
        auto np = p->copy();
        m_ptr.reset(np);
        return np;
    }
    return p;
}

} } // namespace actor
} // namespace boost::detail
