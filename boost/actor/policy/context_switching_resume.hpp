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

#ifndef BOOST_ACTOR_CONTEXT_SWITCHING_ACTOR_HPP
#define BOOST_ACTOR_CONTEXT_SWITCHING_ACTOR_HPP

#include "boost/actor/config.hpp"
#include "boost/actor/resumable.hpp"
#include "boost/actor/mailbox_element.hpp"

#include "boost/actor/detail/logging.hpp"
#include "boost/actor/detail/cs_thread.hpp"

#include "boost/actor/policy/resume_policy.hpp"

#include "boost/actor/detail/yield_interface.hpp"

namespace boost { namespace actor { class local_actor; } }

namespace boost {
namespace actor {
namespace policy {

/**
 * @brief Context-switching actor implementation.
 * @extends scheduled_actor
 */
class context_switching_resume {

 public:

    // required by detail::cs_thread
    static void trampoline(void* _this);

    // Base must be a mailbox-based actor
    template<class Base, class Derived>
    struct mixin : Base, resumable {

        template<typename... Ts>
        mixin(Ts&&... args)
            : Base(std::forward<Ts>(args)...)
            , m_cs_thread(context_switching_resume::trampoline,
                      static_cast<blocking_actor*>(this)) { }

        void attach_to_scheduler() override {
            this->ref();
        }

        void detach_from_scheduler() override {
            this->deref();
        }

        resumable::resume_result resume(detail::cs_thread* from,
                                        execution_unit* host) override {
            BOOST_ACTOR_REQUIRE(from != nullptr);
            BOOST_ACTOR_LOG_TRACE("");
            this->m_host = host;
            using namespace detail;
            for (;;) {
                switch (call(&m_cs_thread, from)) {
                    case yield_state::done: {
                        return resumable::done;
                    }
                    case yield_state::ready: {
                        // should not happen, since it is simply a waste
                        // of time (switching back-and-forth for no reason)
                        BOOST_ACTOR_LOG_WARNING("context-switching actor returned "
                                         "with yield_state::ready");
                        break;
                    }
                    case yield_state::blocked: {
                        if (static_cast<Derived*>(this)->mailbox().try_block()) {
                            return resumable::resume_later;
                        }
                        break;
                    }
                    case yield_state::invalid: {
                        // must not happen
                        BOOST_ACTOR_CRITICAL("illegal state");
                    }
                }
            }
        }

        detail::cs_thread m_cs_thread;

    };

    template<class Actor>
    void await_ready(Actor* self) {
        BOOST_ACTOR_LOG_TRACE("");
        while (!self->has_next_message()) {
            // will call mailbox().try_block() in resume()
            detail::yield(detail::yield_state::blocked);
        }
    }

 private:

    // members
    detail::cs_thread m_cs_thread;

};

} // namespace policy
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_CONTEXT_SWITCHING_ACTOR_HPP
