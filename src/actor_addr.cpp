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


#include "boost/actor/actor.hpp"
#include "boost/actor/singletons.hpp"
#include "boost/actor/actor_addr.hpp"
#include "boost/actor/local_actor.hpp"

#include "boost/actor/io/middleman.hpp"

#include "boost/actor/detail/raw_access.hpp"

namespace boost {
namespace actor {

namespace {
intptr_t compare_impl(const abstract_actor* lhs, const abstract_actor* rhs) {
    return reinterpret_cast<intptr_t>(lhs) - reinterpret_cast<intptr_t>(rhs);
}
} // namespace <anonymous>

actor_addr::actor_addr(const invalid_actor_addr_t&) : m_ptr(nullptr) { }

actor_addr::actor_addr(abstract_actor* ptr) : m_ptr(ptr) { }

intptr_t actor_addr::compare(const actor_addr& other) const {
    return compare_impl(m_ptr.get(), other.m_ptr.get());
}

intptr_t actor_addr::compare(const abstract_actor* other) const {
    return compare_impl(m_ptr.get(), other);
}

actor_addr actor_addr::operator=(const invalid_actor_addr_t&) {
    m_ptr.reset();
    return *this;
}

actor_id actor_addr::id() const {
    return (m_ptr) ? m_ptr->id() : 0;
}

const node_id& actor_addr::node() const {
    return m_ptr ? m_ptr->node() : *get_middleman()->node();
}

bool actor_addr::is_remote() const {
    return m_ptr ? m_ptr->is_proxy() : false;
}

} // namespace actor
} // namespace boost
