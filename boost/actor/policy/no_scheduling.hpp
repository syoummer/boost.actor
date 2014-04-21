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


#ifndef BOOST_ACTOR_NO_SCHEDULING_HPP
#define BOOST_ACTOR_NO_SCHEDULING_HPP

#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>

#include "boost/actor/logging.hpp"
#include "boost/actor/singletons.hpp"
#include "boost/actor/exit_reason.hpp"

#include "boost/actor/detail/cs_thread.hpp"
#include "boost/actor/util/duration.hpp"
#include "boost/actor/util/scope_guard.hpp"

#include "boost/actor/policy/scheduling_policy.hpp"

#include "boost/actor/detail/actor_registry.hpp"
#include "boost/actor/detail/sync_request_bouncer.hpp"

#include "boost/actor/intrusive/single_reader_queue.hpp"

namespace boost {
namespace actor { namespace policy {

class no_scheduling {

    typedef std::unique_lock<std::mutex> lock_type;

 public:

    typedef std::chrono::high_resolution_clock::time_point timeout_type;

    template<class Actor>
    void enqueue(Actor* self, msg_hdr_cref hdr,
                 any_tuple& msg, execution_unit*) {
        auto ptr = self->new_mailbox_element(hdr, std::move(msg));
        // returns false if mailbox has been closed
        if (!self->mailbox().synchronized_enqueue(m_mtx, m_cv, ptr)) {
            if (hdr.id.is_request()) {
                detail::sync_request_bouncer srb{self->exit_reason()};
                srb(hdr.sender, hdr.id);
            }
        }
    }

    template<class Actor>
    void launch(Actor* self, execution_unit*) {
        BOOST_ACTOR_REQUIRE(self != nullptr);
        BOOST_ACTOR_PUSH_AID(self->id());
        BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(self));
        intrusive_ptr<Actor> mself{self};
        self->attach_to_scheduler();
        std::thread([=] {
            BOOST_ACTOR_PUSH_AID(mself->id());
            BOOST_ACTOR_LOG_TRACE("");
            detail::cs_thread fself;
            for (;;) {
                if (mself->resume(&fself, nullptr) == resumable::done) {
                    return;
                }
                // await new data before resuming actor
                await_data(mself.get());
                BOOST_ACTOR_REQUIRE(self->mailbox().blocked() == false);
            }
            self->detach_from_scheduler();
        }).detach();
    }

    // await_data is being called from no_scheduling (only)

    template<class Actor>
    void await_data(Actor* self) {
        if (self->has_next_message()) return;
        self->mailbox().synchronized_await(m_mtx, m_cv);
    }

    // this additional member function is needed to implement
    // timer_actor (see scheduler.cpp)
    template<class Actor, class TimePoint>
    bool await_data(Actor* self, const TimePoint& tp) {
        if (self->has_next_message()) return true;
        return self->mailbox().synchronized_await(m_mtx, m_cv, tp);
    }

 private:

    std::mutex m_mtx;
    std::condition_variable m_cv;

};

} } // namespace actor
} // namespace boost::policy

#endif // BOOST_ACTOR_NO_SCHEDULING_HPP
