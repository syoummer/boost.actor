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


#ifndef REMOTE_ACTOR_PROXY_HPP
#define REMOTE_ACTOR_PROXY_HPP

#include "boost/actor/extend.hpp"
#include "boost/actor/actor_proxy.hpp"

#include "boost/actor/mixin/memory_cached.hpp"

#include "boost/actor/detail/single_reader_queue.hpp"

namespace boost {
namespace actor {
namespace detail {

class memory;

} // namespace detail
} // namespace actor
} // namespace boost

namespace boost {
namespace actor_io {

class middleman;

class sync_request_info : public actor::extend<actor::memory_managed>::
                                 with<actor::mixin::memory_cached> {

    friend class actor::detail::memory;

 public:

    typedef sync_request_info* pointer;

    ~sync_request_info();

    pointer           next;   // intrusive next pointer
    actor::actor_addr sender; // points to the sender of the message
    actor::message_id mid;    // sync message ID

 private:

    sync_request_info(actor::actor_addr sptr, actor::message_id id);

};

class remote_actor_proxy : public actor::actor_proxy {

    typedef actor_proxy super;

 public:

    remote_actor_proxy(actor::actor_id mid,
                       actor::node_id pinfo,
                       actor::actor parent);

    void enqueue(const actor::actor_addr&,
                 actor::message_id,
                 actor::message,
                 actor::execution_unit*) override;

    void link_to(const actor::actor_addr& other) override;

    void unlink_from(const actor::actor_addr& other) override;

    bool remove_backlink(const actor::actor_addr& to) override;

    bool establish_backlink(const actor::actor_addr& to) override;

    void local_link_to(const actor::actor_addr& other) override;

    void local_unlink_from(const actor::actor_addr& other) override;

    void kill_proxy(uint32_t reason) override;

 protected:

    ~remote_actor_proxy();

 private:

    void forward_msg(const actor::actor_addr& sender,
                     actor::message_id mid,
                     actor::message msg);

    actor::actor m_parent;

};

using remote_actor_proxy_ptr = intrusive_ptr<remote_actor_proxy>;

} // namespace actor_io
} // namespace boost

#endif // remote_actor_proxy_HPP
