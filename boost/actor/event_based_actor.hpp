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
