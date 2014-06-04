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
#include <thread>

#include "boost/asio.hpp"

#include "boost/actor/fwd.hpp"
#include "boost/actor/node_id.hpp"
#include "boost/actor/actor_namespace.hpp"
#include "boost/actor/detail/singletons.hpp"

#include "boost/actor_io/peer.hpp"
#include "boost/actor_io/peer_acceptor.hpp"

namespace boost {
namespace actor_io {

/**
 * @brief Multiplexes asynchronous IO.
 */
class middleman : public actor::actor_namespace::backend
                , public actor::detail::abstract_singleton {

    friend class boost::actor::detail::singletons;

 public:

    using node_id = actor::node_id;

    /**
     * @brief Get middleman instance.
     */
    static middleman* instance();

    ~middleman();

    /**
     * @brief Runs @p fun in the event loop of the middleman.
     * @note This member function is thread-safe.
     */
    template<typename F>
    void run_later(F fun) {
        if (m_thread.get_id() == std::this_thread::get_id()) fun();
        else m_backend.dispatch(fun);
    }

    /**
     * @brief Adds @p mgr to the list of known network IO device managers.
     */
    void add(const network::manager_ptr& mgr);

    /**
     * @brief Removes @p mgr from the list of known network IO device managers.
     */
    void remove(const network::manager_ptr& mgr);

    /**
     * @brief Tries to register a new peer, i.e., a new node in the network.
     *        Returns false if there is already a connection to @p node,
     *        otherwise true.
     */
    bool register_peer(const node_id& node, const peer_ptr& ptr);

    /**
     * @brief Returns the peer associated with given node id.
     */
    peer_ptr get_peer(const node_id& node);

    /**
     * @brief This callback is used by peer implementations to
     *        invoke cleanup code when disposed.
     */
    void del_peer(const peer_ptr& pptr);

    /**
     * @brief Announces a new routing path to @p destination via @p hop.
     */
    void announce_route(const node_id& hop, const node_id& destination);

    /**
     * @brief Delivers a message to given node.
     */
    void dispatch(const node_id& node, actor::msg_hdr_cref hdr, actor::message msg);

    /**
     * @brief Delivers a message to given node.
     * @warning Must not be called from outside the middleman's event loop.
     */
    void dispatch(const node_id& node, const void* buf, size_t buf_len);

    /**
     * @brief This callback is invoked by {@link peer} implementations
     *        and causes the middleman to disconnect from the node.
     */
    void last_proxy_exited(const peer_ptr& pptr);

    /**
     * @brief Returns the namespace that contains all remote actors
     *        connected to this middleman.
     */
    inline actor::actor_namespace& get_namespace();

    /**
     * @brief Creates a new proxy instance.
     */
    actor::actor_proxy_ptr make_proxy(const actor::node_id_ptr&,
                                      actor::actor_id) override;

    /**
     * @brief Registers a proxy instance.
     */
    void register_proxy(const node_id&, actor::actor_id) override;

    /**
     * @brief Returns the node ID of this middleman.
     */
    const actor::node_id& node() const;

    /**
     * @brief Returns a pointer to the node ID of this middleman.
     */
    const actor::node_id_ptr& node_ptr() const override;

    /**
     * @brief Returns the IO backend used by this middleman.
     */
    inline boost::asio::io_service& backend() {
        return m_backend;
    }

    /** @cond PRIVATE */

    // destroys uninitialized instances
    void dispose() override;

    // destroys an initialized singleton
    void destroy() override;

    // initializes a singleton
    void initialize() override;

    /** @endcond */

 private:

    middleman();

    peer* select_peer(const node_id& nid);

    using routing_table = std::multimap<node_id /* dest */, node_id /* hop */>;

    network::multiplexer           m_backend;    // networking backend
    network::supervisor*           m_supervisor; // keeps backend busy
    actor::actor_namespace         m_namespace;  // manages proxies
    actor::node_id_ptr             m_node;       // identifies this node
    std::set<network::manager_ptr> m_managers;   // keeps managers alive
    std::map<node_id, peer_ptr>    m_peers;      // known peers
    routing_table                  m_routes;     // stores non-direct routes
    routing_table                  m_blacklist;  // stores invalidated routes
    std::thread                    m_thread;     // runs the backend

};

inline actor::actor_namespace& middleman::get_namespace() {
    return m_namespace;
}

} // namespace actor_io
} // namespace boost

#endif // MIDDLEMAN_HPP
