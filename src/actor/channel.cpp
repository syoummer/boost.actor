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


#include "boost/actor/actor.hpp"
#include "boost/actor/group.hpp"
#include "boost/actor/channel.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/actor_cast.hpp"

namespace boost {
namespace actor {

channel::channel(const actor& other) : m_ptr(actor_cast<abstract_channel_ptr>(other)) { }

channel::channel(const group& other) : m_ptr(actor_cast<abstract_channel_ptr>(other)) { }

channel::channel(const invalid_actor_t&) : m_ptr(nullptr) { }

channel::channel(const invalid_group_t&) : m_ptr(nullptr) { }

intptr_t channel::compare(const abstract_channel* lhs, const abstract_channel* rhs) {
    return reinterpret_cast<intptr_t>(lhs) - reinterpret_cast<intptr_t>(rhs);
}

channel::channel(abstract_channel* ptr) : m_ptr(ptr) { }

intptr_t channel::compare(const channel& other) const {
    return compare(m_ptr.get(), other.m_ptr.get());
}

intptr_t channel::compare(const actor& other) const {
    return compare(m_ptr.get(), actor_cast<abstract_actor_ptr>(other).get());
}

intptr_t channel::compare(const abstract_channel* other) const {
    return compare(m_ptr.get(), other);
}

} // namespace actor
} // namespace boost
