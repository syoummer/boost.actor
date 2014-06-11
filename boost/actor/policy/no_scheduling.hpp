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


#ifndef BOOST_ACTOR_NO_SCHEDULING_HPP
#define BOOST_ACTOR_NO_SCHEDULING_HPP

#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>

#include "boost/actor/duration.hpp"
#include "boost/actor/exit_reason.hpp"

#include "boost/actor/policy/scheduling_policy.hpp"

#include "boost/actor/detail/logging.hpp"
#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/scope_guard.hpp"
#include "boost/actor/detail/actor_registry.hpp"
#include "boost/actor/detail/sync_request_bouncer.hpp"
#include "boost/actor/detail/single_reader_queue.hpp"

namespace boost {
namespace actor { namespace policy {

class no_scheduling {

    typedef std::unique_lock<std::mutex> lock_type;

 public:

    typedef std::chrono::high_resolution_clock::time_point timeout_type;

    template<class Actor>
    void enqueue(Actor* self, const actor_addr& sender,
                 message_id mid, message& msg, execution_unit*) {
        auto ptr = self->new_mailbox_element(sender, mid, std::move(msg));
        // returns false if mailbox has been closed
        if (!self->mailbox().synchronized_enqueue(m_mtx, m_cv, ptr)) {
            if (mid.is_request()) {
                detail::sync_request_bouncer srb{self->exit_reason()};
                srb(sender, mid);
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
            for (;;) {
                if (mself->resume(nullptr) == resumable::done) {
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
