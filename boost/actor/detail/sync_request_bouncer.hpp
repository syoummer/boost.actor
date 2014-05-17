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


#ifndef BOOST_ACTOR_SYNC_REQUEST_BOUNCER_HPP
#define BOOST_ACTOR_SYNC_REQUEST_BOUNCER_HPP

#include <cstdint>

namespace boost {
namespace actor {

class actor_addr;
class message_id;
class local_actor;
class mailbox_element;

} // namespace actor
} // namespace boost

namespace boost {
namespace actor {
namespace detail {

struct sync_request_bouncer {
    std::uint32_t rsn;
    explicit sync_request_bouncer(std::uint32_t r);
    void operator()(const actor_addr& sender, const message_id& mid) const;
    void operator()(const mailbox_element& e) const;
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_SYNC_REQUEST_BOUNCER_HPP
