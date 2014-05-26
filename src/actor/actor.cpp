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


#include <utility>

#include "boost/actor/actor.hpp"
#include "boost/actor/channel.hpp"
#include "boost/actor/actor_addr.hpp"
#include "boost/actor/actor_proxy.hpp"
#include "boost/actor/local_actor.hpp"
#include "boost/actor/blocking_actor.hpp"
#include "boost/actor/event_based_actor.hpp"

namespace boost {
namespace actor {

actor::actor(const invalid_actor_t&) : m_ptr(nullptr) { }

actor::actor(abstract_actor* ptr) : m_ptr(ptr) { }

actor& actor::operator=(const invalid_actor_t&) {
    m_ptr.reset();
    return *this;
}

intptr_t actor::compare(const actor& other) const {
    return channel::compare(m_ptr.get(), other.m_ptr.get());
}

intptr_t actor::compare(const actor_addr& other) const {
    return static_cast<ptrdiff_t>(m_ptr.get() - other.m_ptr.get());
}

void actor::swap(actor& other) {
    m_ptr.swap(other.m_ptr);
}

actor_addr actor::address() const {
    return m_ptr ? m_ptr->address() : actor_addr{};
}

} // namespace actor
} // namespace boost
