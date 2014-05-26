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


#ifndef BOOST_ACTOR_MESSAGE_HEADER_HPP
#define BOOST_ACTOR_MESSAGE_HEADER_HPP

#include "boost/actor/channel.hpp"
#include "boost/actor/actor_addr.hpp"
#include "boost/actor/message_id.hpp"
#include "boost/actor/message_priority.hpp"

namespace boost {
namespace actor {

class message;

/**
 * @brief Encapsulates information about sender, receiver and (synchronous)
 *        message ID of a message. The message itself is usually an message.
 */
class message_header {

 public:

    actor_addr       sender;
    channel          receiver;
    message_id       id;

    /**
     * @brief An invalid message header without receiver or sender;
     **/
    message_header() = default;

    /**
     * @brief Creates a message header with <tt>receiver = dest</tt> and
     *        <tt>sender = source</tt>.
     */
    message_header(actor_addr source,
                   channel dest,
                   message_id mid = message_id::invalid);

    void deliver(message msg) const;

};

/**
 * @brief Convenience typedef.
 */
typedef const message_header& msg_hdr_cref;

bool operator==(msg_hdr_cref lhs, msg_hdr_cref rhs);

bool operator!=(msg_hdr_cref lhs, msg_hdr_cref rhs);

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_MESSAGE_HEADER_HPP
