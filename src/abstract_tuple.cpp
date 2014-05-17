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
