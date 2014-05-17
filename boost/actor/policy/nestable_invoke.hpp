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


#ifndef BOOST_ACTOR_THREADED_HPP
#define BOOST_ACTOR_THREADED_HPP

#include <mutex>
#include <chrono>
#include <condition_variable>

#include "boost/actor/exit_reason.hpp"
#include "boost/actor/mailbox_element.hpp"

#include "boost/actor/detail/sync_request_bouncer.hpp"
#include "boost/actor/detail/single_reader_queue.hpp"

#include "boost/actor/policy/invoke_policy.hpp"

namespace boost { namespace actor {
namespace detail { class receive_policy; } } }

namespace boost {
namespace actor {
namespace policy {

class nestable_invoke : public invoke_policy<nestable_invoke> {

 public:

    inline bool hm_should_skip(mailbox_element* node) {
        return node->marked;
    }

    template<class Actor>
    inline mailbox_element* hm_begin(Actor* self, mailbox_element* node) {
        auto previous = self->current_node();
        self->current_node(node);
        self->push_timeout();
        node->marked = true;
        return previous;
    }

    template<class Actor>
    inline void hm_cleanup(Actor* self, mailbox_element* previous) {
        self->current_node()->marked = false;
        self->current_node(previous);
    }

    template<class Actor>
    inline void hm_revert(Actor* self, mailbox_element* previous) {
        self->current_node()->marked = false;
        self->current_node(previous);
        self->pop_timeout();
    }

};

} // namespace policy
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_THREADED_HPP
