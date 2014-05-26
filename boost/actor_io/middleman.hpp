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
class middleman : public actor::actor_namespace::backend {

    friend class boost::actor::detail::singletons;

 public:

    /**
     * @brief Get middleman instance.
     */
    static middleman* instance();

    middleman();

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
    bool register_peer(const actor::node_id& node, const peer_ptr& ptr);

    /**
     * @brief Returns the peer associated with given node id.
     */
    peer_ptr get_peer(const actor::node_id& node);

    /**
     * @brief This callback is used by peer implementations to
     *        invoke cleanup code when disposed.
     */
    void del_peer(const peer_ptr& pptr);

    /**
     * @brief Delivers a message to given node.
     */
    void dispatch(const actor::node_id& node,
                  actor::msg_hdr_cref hdr,
                  actor::message msg);

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
    void register_proxy(const actor::node_id&, actor::actor_id) override;

    /**
     * @brief Returns the node of this middleman.
     */
    const actor::node_id_ptr& node() const override;

    /**
     * @brief Returns the IO backend used by this middleman.
     */
    inline boost::asio::io_service& backend() {
        return m_backend;
    }

    /**
     * @brief Buffer type used for asynchronous IO.
     */
    typedef std::vector<char> buffer_type;

    /**
     * @brief Returns a buffer for asynchronous IO. This buffer is owned by
     *        the middleman and must not be deleted.
     * @warning Call *only* from completion handlers invoked from the backend.
     */
    buffer_type* acquire_buffer();

    /**
     * @brief Allows the middleman to re-use @p buf in a subsequent
     *        in a subsequent call to @p acquire_buffer.
     * @warning Call *only* from completion handlers invoked from the backend.
     */
    void release_buffer(buffer_type* buf);

    inline std::thread::id thread_id() {
        return m_thread.get_id();
    }

 private:

    network::multiplexer m_backend;
    network::supervisor* m_supervisor;

    // creates a middleman instance
    static inline middleman* create_singleton() {
        return new middleman;
    }

    // destroys uninitialized instances
    inline void dispose() { delete this; }

    // destroys an initialized singleton
    void destroy();

    // initializes a singleton
    void initialize();

    // each middleman defines its own namespace
    actor::actor_namespace m_namespace;

    // the node id of this middleman
    actor::node_id_ptr m_node;

    std::set<network::manager_ptr> m_managers;

    std::map<boost::actor::node_id, peer_ptr> m_peers;

    std::thread m_thread;

};

inline actor::actor_namespace& middleman::get_namespace() {
    return m_namespace;
}

} // namespace actor_io
} // namespace boost

#endif // MIDDLEMAN_HPP
