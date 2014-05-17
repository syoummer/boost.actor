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


#ifndef BOOST_ACTOR_ACTOR_PROXY_HPP
#define BOOST_ACTOR_ACTOR_PROXY_HPP

#include "boost/actor/abstract_actor.hpp"
#include "boost/actor/message_header.hpp"

namespace boost {
namespace actor {

class actor_proxy_cache;

/**
 * @brief Represents a remote actor.
 * @extends abstract_actor
 */
class actor_proxy : public abstract_actor {

    typedef abstract_actor super;

 public:

    ~actor_proxy();

    /**
     * @brief Establishes a local link state that's not synchronized back
     *        to the remote instance.
     */
    virtual void local_link_to(const actor_addr& other) = 0;

    /**
     * @brief Removes a local link state.
     */
    virtual void local_unlink_from(const actor_addr& other) = 0;

    /**
     * @brief Delivers given message via this proxy instance.
     *
     * This function is meant to give the proxy the opportunity to keep track
     * of synchronous communication or perform other bookkeeping if needed.
     * The member function is called by the protocol from inside the
     * middleman's thread.
     * @note This function is guaranteed to be called non-concurrently.
     */
    virtual void deliver(msg_hdr_cref hdr, message msg) = 0;

 protected:

    actor_proxy(actor_id mid);

};

/**
 * @brief A smart pointer to an {@link actor_proxy} instance.
 * @relates actor_proxy
 */
typedef intrusive_ptr<actor_proxy> actor_proxy_ptr;

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_ACTOR_PROXY_HPP
