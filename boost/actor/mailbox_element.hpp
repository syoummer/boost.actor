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


#ifndef BOOST_ACTOR_RECURSIVE_QUEUE_NODE_HPP
#define BOOST_ACTOR_RECURSIVE_QUEUE_NODE_HPP

#include <cstdint>

#include "boost/actor/extend.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/actor_addr.hpp"
#include "boost/actor/message_id.hpp"
#include "boost/actor/ref_counted.hpp"
#include "boost/actor/message_header.hpp"

#include "boost/actor/mixin/memory_cached.hpp"

// needs access to constructor + destructor to initialize m_dummy_node
namespace boost {
namespace actor {

class local_actor;

class mailbox_element : public extend<memory_managed>::
                               with<mixin::memory_cached> {

    friend class local_actor;
    friend class detail::memory;

 public:

    mailbox_element* next;   // intrusive next pointer
    bool             marked; // denotes if this node is currently processed
    actor_addr       sender;
    message        msg;    // 'content field'
    message_id       mid;

    ~mailbox_element();

    mailbox_element(mailbox_element&&) = delete;
    mailbox_element(const mailbox_element&) = delete;
    mailbox_element& operator=(mailbox_element&&) = delete;
    mailbox_element& operator=(const mailbox_element&) = delete;

    template<typename T>
    inline static mailbox_element* create(msg_hdr_cref hdr, T&& data) {
        return detail::memory::create<mailbox_element>(hdr, std::forward<T>(data));
    }

 private:

    mailbox_element() = default;

    mailbox_element(msg_hdr_cref hdr, message data);

};

typedef std::unique_ptr<mailbox_element, detail::disposer>
        unique_mailbox_element_pointer;

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_RECURSIVE_QUEUE_NODE_HPP
