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


#include "boost/actor/continue_helper.hpp"
#include "boost/actor/event_based_actor.hpp"

namespace boost {
namespace actor {

continue_helper::continue_helper(message_id mid, local_actor* self)
        : m_mid(mid), m_self(self) { }

continue_helper& continue_helper::continue_with(behavior::continuation_fun f) {
    auto ref_opt = m_self->sync_handler(m_mid);
    if (ref_opt) {
        behavior cpy = *ref_opt;
        *ref_opt = cpy.add_continuation(std::move(f));
    }
    else { BOOST_ACTOR_LOG_ERROR("failed to add continuation"); }
    return *this;
}

} // namespace actor
} // namespace boost
