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


#include "boost/actor/message.hpp"
#include "boost/actor/message_header.hpp"

namespace boost {
namespace actor {

message_header::message_header(actor_addr source,
                               channel dest,
                               message_id mid)
: sender(source), receiver(dest), id(mid) { }

bool operator==(const message_header& lhs, const message_header& rhs) {
    return    lhs.sender == rhs.sender
           && lhs.receiver == rhs.receiver
           && lhs.id == rhs.id;
}

bool operator!=(const message_header& lhs, const message_header& rhs) {
    return !(lhs == rhs);
}

void message_header::deliver(message msg) const {
    if (receiver) receiver->enqueue(*this, std::move(msg), nullptr);
}

} // namespace actor
} // namespace boost::network
