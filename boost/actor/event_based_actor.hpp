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


#ifndef BOOST_ACTOR_UNTYPED_ACTOR_HPP
#define BOOST_ACTOR_UNTYPED_ACTOR_HPP

#include "boost/actor/on.hpp"
#include "boost/actor/extend.hpp"
#include "boost/actor/logging.hpp"
#include "boost/actor/local_actor.hpp"
#include "boost/actor/response_handle.hpp"

#include "boost/actor/mixin/sync_sender.hpp"
#include "boost/actor/mixin/mailbox_based.hpp"
#include "boost/actor/mixin/behavior_stack_based.hpp"

#include "boost/actor/detail/response_handle_util.hpp"

namespace boost {
namespace actor {

/**
 * @brief A cooperatively scheduled, event-based actor implementation.
 *
 * This is the recommended base class for user-defined actors and is used
 * implicitly when spawning functor-based actors without the
 * {@link blocking_api} flag.
 *
 * @extends local_actor
 */
class event_based_actor
        : public extend<local_actor, event_based_actor>::
                 with<mixin::mailbox_based,
                      mixin::behavior_stack_based<behavior>::impl,
                      mixin::sync_sender<nonblocking_response_handle_tag>::impl> {

 public:

    event_based_actor();

    ~event_based_actor();

 protected:

    /**
     * @brief Returns the initial actor behavior.
     */
    virtual behavior make_behavior() = 0;

    /**
     * @brief Forwards the last received message to @p whom.
     */
    void forward_to(const actor& whom);

    bool m_initialized;

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_UNTYPED_ACTOR_HPP
