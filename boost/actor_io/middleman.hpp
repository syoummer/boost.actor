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


#ifndef MIDDLEMAN_HPP
#define MIDDLEMAN_HPP

#include <map>
#include <vector>
#include <memory>
#include <functional>

#include "boost/actor/node_id.hpp"
#include "boost/actor/fwd.hpp"
#include "boost/actor/actor_namespace.hpp"

namespace boost { namespace actor { namespace detail { class singletons; } } }

namespace boost {
namespace actor {

class actor_proxy;
typedef intrusive_ptr<actor_proxy> actor_proxy_ptr;

} // namespace actor
} // namespace boost

namespace boost {
namespace actor_io {

class peer;
class continuable;
class input_stream;
class peer_acceptor;
class output_stream;
class middleman_event_handler;

typedef intrusive_ptr<input_stream> input_stream_ptr;
typedef intrusive_ptr<output_stream> output_stream_ptr;

/**
 * @brief Multiplexes asynchronous IO.
 * @note No member function except for @p run_later is safe to call from
 *       outside the event loop.
 */
class middleman {

    friend class boost::actor::detail::singletons;

 public:

    /**
     * @brief Get middleman instance.
     */
    static middleman* instance();

    virtual ~middleman();

    /**
     * @brief Runs @p fun in the event loop of the middleman.
     * @note This member function is thread-safe.
     */
    virtual void run_later(std::function<void()> fun) = 0;

    /**
     * @brief Removes @p ptr from the list of active writers.
     */
    void stop_writer(continuable* ptr);

    /**
     * @brief Adds @p ptr to the list of active writers.
     */
    void continue_writer(continuable* ptr);

    /**
     * @brief Checks wheter @p ptr is an active writer.
     * @warning This member function is not thread-safe.
     */
    bool has_writer(continuable* ptr);

    /**
     * @brief Removes @p ptr from the list of active readers.
     * @warning This member function is not thread-safe.
     */
    void stop_reader(continuable* ptr);

    /**
     * @brief Adds @p ptr to the list of active readers.
     * @warning This member function is not thread-safe.
     */
    void continue_reader(continuable* ptr);

    /**
     * @brief Checks wheter @p ptr is an active reader.
     * @warning This member function is not thread-safe.
     */
    bool has_reader(continuable* ptr);

    /**
     * @brief Tries to register a new peer, i.e., a new node in the network.
     *        Returns false if there is already a connection to @p node,
     *        otherwise true.
     */
    virtual bool register_peer(const actor::node_id& node, peer* ptr) = 0;

    /**
     * @brief Returns the peer associated with given node id.
     */
    virtual peer* get_peer(const actor::node_id& node) = 0;

    /**
     * @brief This callback is used by peer_acceptor implementations to
     *        invoke cleanup code when disposed.
     */
    virtual void del_acceptor(peer_acceptor* ptr) = 0;

    /**
     * @brief This callback is used by peer implementations to
     *        invoke cleanup code when disposed.
     */
    virtual void del_peer(peer* ptr) = 0;

    /**
     * @brief Delivers a message to given node.
     */
    virtual void deliver(const actor::node_id& node,
                         actor::msg_hdr_cref hdr,
                         actor::message msg         ) = 0;

    /**
     * @brief This callback is invoked by {@link peer} implementations
     *        and causes the middleman to disconnect from the node.
     */
    virtual void last_proxy_exited(peer* ptr) = 0;

    /**
     *
     */
    virtual void new_peer(const input_stream_ptr& in,
                          const output_stream_ptr& out,
                          const actor::node_id_ptr& node = nullptr) = 0;

    /**
     * @brief Adds a new acceptor for incoming connections to @p pa
     *        to the event loop of the middleman.
     * @note This member function is thread-safe.
     */
    virtual void register_acceptor(const actor::actor_addr& pa,
                                   peer_acceptor* ptr) = 0;

    /**
     * @brief Returns the namespace that contains all remote actors
     *        connected to this middleman.
     */
    inline actor::actor_namespace& get_namespace();

    /**
     * @brief Returns the node of this middleman.
     */
    inline const actor::node_id_ptr& node() const;

 protected:

    // creates a middleman instance
    static middleman* create_singleton();

    // destroys uninitialized instances
    inline void dispose() { delete this; }

    // destroys an initialized singleton
    virtual void destroy() = 0;

    // initializes a singleton
    virtual void initialize() = 0;

    // each middleman defines its own namespace
    actor::actor_namespace m_namespace;

    // the node id of this middleman
    actor::node_id_ptr m_node;

    std::unique_ptr<middleman_event_handler> m_handler;

};

inline actor::actor_namespace& middleman::get_namespace() {
    return m_namespace;
}

const actor::node_id_ptr& middleman::node() const {
    BOOST_ACTOR_REQUIRE(m_node != nullptr);
    return m_node;
}

} // namespace actor_io
} // namespace boost

#endif // MIDDLEMAN_HPP
