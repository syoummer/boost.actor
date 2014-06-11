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


#include "boost/actor/scheduler.hpp"
#include "boost/actor/message_id.hpp"
#include "boost/actor/event_based_actor.hpp"

#include "boost/actor/detail/singletons.hpp"

namespace boost {
namespace actor {

event_based_actor::event_based_actor() : m_initialized(false) { }

event_based_actor::~event_based_actor() { }

void event_based_actor::forward_to(const actor& whom) {
    forward_message(whom, message_priority::normal);
}

behavior event_based_actor::functor_based::make_behavior() {
    return m_make_behavior(this);
}

} // namespace actor
} // namespace boost
