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


#ifndef BOOST_ACTOR_POLICY_MIDDLEMAN_SCHEDULING_HPP
#define BOOST_ACTOR_POLICY_MIDDLEMAN_SCHEDULING_HPP

#include <utility>

#include "boost/actor/singletons.hpp"
#include "boost/intrusive_ptr.hpp"

#include "boost/actor/io/middleman.hpp"

#include "boost/actor/policy/cooperative_scheduling.hpp"

namespace boost {
namespace actor {
namespace policy {

// Actor must implement invoke_message
class middleman_scheduling {

 public:

    template<class Actor>
    class continuation {

     public:

        typedef intrusive_ptr<Actor> pointer;

        continuation(pointer ptr, msg_hdr_cref hdr, message&& msg)
        : m_self(std::move(ptr)), m_hdr(hdr), m_data(std::move(msg)) { }

        inline void operator()() const {
            m_self->invoke_message(m_hdr, std::move(m_data));
        }

     private:

        pointer        m_self;
        message_header m_hdr;
        message      m_data;

    };

    using timeout_type = int;

    // this does return nullptr
    template<class Actor, typename F>
    inline void fetch_messages(Actor*, F) {
        // clients cannot fetch messages
    }

    template<class Actor, typename F>
    inline void fetch_messages(Actor* self, F cb, timeout_type) {
        // a call to this call is always preceded by init_timeout,
        // which will trigger a timeout message
        fetch_messages(self, cb);
    }

    template<class Actor>
    inline void launch(Actor*) {
        // nothing to do
    }

    template<class Actor>
    void enqueue(Actor* self, msg_hdr_cref hdr, message& msg) {
        get_middleman()->run_later(continuation<Actor>{self, hdr, std::move(msg)});
    }

};

} // namespace policy
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_POLICY_MIDDLEMAN_SCHEDULING_HPP
