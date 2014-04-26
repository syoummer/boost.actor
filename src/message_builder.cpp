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


#include <vector>

#include "boost/actor/message_builder.hpp"
#include "boost/actor/uniform_type_info.hpp"

namespace boost {
namespace actor {

namespace {

class dynamic_msg_data : public detail::message_data {

    typedef message_data super;

 public:

    using message_data::const_iterator;

    dynamic_msg_data(const dynamic_msg_data& other) : super(true){
        for (auto& d : other.m_data) {
            m_data.push_back(d->copy());
        }
    }

    dynamic_msg_data(std::vector<uniform_value>&& data)
            : super(true), m_data(std::move(data)) { }

    const void* at(size_t pos) const override {
        BOOST_ACTOR_REQUIRE(pos < size());
        return m_data[pos]->val;
    }

    void* mutable_at(size_t pos) override {
        BOOST_ACTOR_REQUIRE(pos < size());
        return m_data[pos]->val;
    }

    size_t size() const override {
        return m_data.size();
    }

    dynamic_msg_data* copy() const override {
        return new dynamic_msg_data(*this);
    }

    const uniform_type_info* type_at(size_t pos) const override {
        BOOST_ACTOR_REQUIRE(pos < size());
        return m_data[pos]->ti;
    }

    const std::string* tuple_type_names() const override {
        return nullptr; // get_tuple_type_names(*this);
    }

 private:

    std::vector<uniform_value> m_data;

};

} // namespace <anonymous>

message_builder& message_builder::append(uniform_value what) {
    m_elements.push_back(std::move(what));
    return *this;
}

message message_builder::to_message() {
    return message{new dynamic_msg_data(std::move(m_elements))};
}

} // namespace actor
} // namespace boost
