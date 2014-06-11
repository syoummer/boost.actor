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

#include "boost/actor/local_actor.hpp"
#include "boost/actor/response_promise.hpp"

using std::move;

namespace boost {
namespace actor {

response_promise::response_promise(const actor_addr& from,
                                   const actor_addr& to,
                                   const message_id& id)
: m_from(from), m_to(to), m_id(id) {
    BOOST_ACTOR_REQUIRE(id.is_response() || !id.valid());
}

void response_promise::deliver(message msg) {
    if (m_to) {
        auto to = actor_cast<abstract_actor_ptr>(m_to);
        auto from = actor_cast<abstract_actor_ptr>(m_from);
        to->enqueue(m_from, m_id, move(msg), from->m_host);
        m_to = invalid_actor_addr;
    }
}

} // namespace actor
} // namespace boost
