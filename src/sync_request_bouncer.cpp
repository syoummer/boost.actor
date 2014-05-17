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


#include "boost/actor/atom.hpp"
#include "boost/actor/config.hpp"
#include "boost/actor/message_id.hpp"
#include "boost/actor/exit_reason.hpp"
#include "boost/actor/mailbox_element.hpp"
#include "boost/actor/system_messages.hpp"

#include "boost/actor/detail/raw_access.hpp"
#include "boost/actor/detail/sync_request_bouncer.hpp"

namespace boost {
namespace actor {
namespace detail {

sync_request_bouncer::sync_request_bouncer(std::uint32_t r)
: rsn(r == exit_reason::not_exited ? exit_reason::normal : r) { }

void sync_request_bouncer::operator()(const actor_addr& sender,
                                      const message_id& mid) const {
    BOOST_ACTOR_REQUIRE(rsn != exit_reason::not_exited);
    if (sender && mid.is_request()) {
        auto ptr = detail::raw_access::get(sender);
        ptr->enqueue({invalid_actor_addr, ptr, mid.response_id()},
                     make_message(sync_exited_msg{sender, rsn}),
                     // TODO: this breaks out of the execution unit
                     nullptr);
    }
}

void sync_request_bouncer::operator()(const mailbox_element& e) const {
    (*this)(e.sender, e.mid);
}

} } // namespace actor
} // namespace boost::detail
