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


#ifndef BOOST_ACTOR_COOPERATIVE_SCHEDULING_HPP
#define BOOST_ACTOR_COOPERATIVE_SCHEDULING_HPP

#include <atomic>

#include "boost/actor/message.hpp"
#include "boost/actor/scheduler.hpp"
#include "boost/actor/message_header.hpp"

#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/yield_interface.hpp"
#include "boost/actor/detail/single_reader_queue.hpp"

namespace boost {
namespace actor { namespace policy {

class cooperative_scheduling {

 public:

    using timeout_type = int;

    template<class Actor>
    inline void launch(Actor* self, execution_unit* host) {
        // detached in scheduler::worker::run
        self->attach_to_scheduler();
        if (host) host->exec_later(self);
        else detail::singletons::get_scheduling_coordinator()->enqueue(self);
    }

    template<class Actor>
    void enqueue(Actor* self, msg_hdr_cref hdr,
                 message& msg, execution_unit* host) {
        auto e = self->new_mailbox_element(hdr, std::move(msg));
        switch (self->mailbox().enqueue(e)) {
            case detail::enqueue_result::unblocked_reader: {
                // re-schedule actor
                if (host) host->exec_later(self);
                else detail::singletons::get_scheduling_coordinator()
                     ->enqueue(self);
                break;
            }
            case detail::enqueue_result::queue_closed: {
                if (hdr.id.is_request()) {
                    detail::sync_request_bouncer f{self->exit_reason()};
                    f(hdr.sender, hdr.id);
                }
                break;
            }
            case detail::enqueue_result::success:
                // enqueued to a running actors' mailbox; nothing to do
                break;
        }
    }

};

} } // namespace actor
} // namespace boost::policy

#endif // BOOST_ACTOR_COOPERATIVE_SCHEDULING_HPP
