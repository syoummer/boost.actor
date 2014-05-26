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


#include "boost/actor/policy.hpp"
#include "boost/actor/scoped_actor.hpp"

#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/proper_actor.hpp"
#include "boost/actor/detail/actor_registry.hpp"

namespace boost {
namespace actor {

namespace {

struct impl : blocking_actor {
    void act() override { }
};

blocking_actor* alloc() {
    using namespace policy;
    return new detail::proper_actor<impl,
                                    policies<no_scheduling,
                                             not_prioritizing,
                                             no_resume,
                                             nestable_invoke>>;
}

} // namespace <anonymous>

void scoped_actor::init(bool hide_actor) {
    m_hidden = hide_actor;
    m_self.reset(alloc());
    if (!m_hidden) {
        detail::singletons::get_actor_registry()->inc_running();
        m_prev = BOOST_ACTOR_SET_AID(m_self->id());
    }
}

scoped_actor::scoped_actor() {
    init(false);
}

scoped_actor::scoped_actor(bool hide_actor) {
    init(hide_actor);
}

scoped_actor::~scoped_actor() {
    if (!m_hidden) {
        detail::singletons::get_actor_registry()->dec_running();
        BOOST_ACTOR_SET_AID(m_prev);
    }
}

} // namespace actor
} // namespace boost
