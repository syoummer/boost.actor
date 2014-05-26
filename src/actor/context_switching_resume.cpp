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


#include "boost/actor/policy/context_switching_resume.hpp"
#ifndef BOOST_ACTOR_DISABLE_CONTEXT_SWITCHING

#include <iostream>

#include "boost/actor/all.hpp"
#include "boost/actor/exception.hpp"

namespace boost {
namespace actor {
namespace policy {

void context_switching_resume::trampoline(void* this_ptr) {
    BOOST_ACTOR_LOGF_TRACE(BOOST_ACTOR_ARG(this_ptr));
    auto self = reinterpret_cast<blocking_actor*>(this_ptr);
    auto shut_actor_down = [self](std::uint32_t reason) {
        if (self->planned_exit_reason() == exit_reason::not_exited) {
            self->planned_exit_reason(reason);
        }
        self->on_exit();
        self->cleanup(self->planned_exit_reason());
    };
    try {
        self->act();
        shut_actor_down(exit_reason::normal);
    }
    catch (actor_exited& e) {
        shut_actor_down(e.reason());
    }
    catch (...) {
        shut_actor_down(exit_reason::unhandled_exception);
    }
    std::atomic_thread_fence(std::memory_order_seq_cst);
    BOOST_ACTOR_LOGF_DEBUG("done, yield() back to execution unit");;
    detail::yield(detail::yield_state::done);
}

} // namespace policy
} // namespace actor
} // namespace boost

#else // ifdef BOOST_ACTOR_DISABLE_CONTEXT_SWITCHING

namespace boost {
namespace actor {
namespace policy {

void context_switching_resume::trampoline(void*) {
    throw std::logic_error("context_switching_resume::trampoline called");
}

} // namespace policy
} // namespace actor
} // namespace boost

#endif // ifdef BOOST_ACTOR_DISABLE_CONTEXT_SWITCHING
