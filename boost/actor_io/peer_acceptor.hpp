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


#ifndef tcp_PEER_ACCEPTOR_HPP
#define tcp_PEER_ACCEPTOR_HPP

#include "boost/actor/actor_addr.hpp"

#include "boost/actor_io/peer.hpp"

namespace boost {
namespace actor_io {

class peer_acceptor : public network::acceptor_manager {

 public:


    virtual ~peer_acceptor();

    using actor_addr = boost::actor::actor_addr;

    /**
     * @brief Creates and starts a new peer acceptor.
     */
    template<class SocketAcceptor>
    class impl;

    template<class SocketAcceptor>
    static impl<SocketAcceptor>* make(middleman* parent,
                                    boost::actor::actor_addr published_actor,
                                    SocketAcceptor accept_sock);

    inline const actor_addr& published_actor() const { return m_aa; }

    network::multiplexer& backend();

 protected:

    void register_at_parent(const network::manager_ptr& self);

    inline middleman* parent() {
        return m_parent;
    }

    peer_acceptor(middleman* parent,
                  actor_addr&& published_actor);

    void handle_io_failure(const network::manager_ptr& self,
                           const std::string& error_description);

    middleman* m_parent;
    actor_addr m_aa;

};

template<class SocketAcceptor>
class peer_acceptor::impl : public peer_acceptor {

    typedef peer_acceptor super;

 public:

    typedef network::acceptor<SocketAcceptor> socket_acceptor;

    template<typename... Ts>
    impl(middleman* mm, Ts&&... args)
    : super(mm, std::forward<Ts>(args)...), m_acceptor(backend()) { }

    typedef asio::basic_stream_socket<typename SocketAcceptor::protocol_type>
            socket_type;

    void new_connection() override {
        BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(this));
        auto pptr = peer::make(m_parent, std::move(m_acceptor.accepted_socket()));
        pptr->start_server(m_aa);
    }

    void io_failure(network::operation, const std::string& err) override {
        BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(this));
        handle_io_failure(this, err);
    }

    void stop_reading() override {
        BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(this));
        m_acceptor.stop();
    }

    void start() {
        BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(this));
        m_acceptor.start(this);
    }

    socket_acceptor& get_acceptor() {
        return m_acceptor;
    }

 private:

    socket_acceptor m_acceptor;

};

template<class SocketAcceptor>
peer_acceptor::impl<SocketAcceptor>* peer_acceptor::make(middleman* mm,
                                                         actor::actor_addr addr,
                                                         SocketAcceptor fd) {
    auto result = new impl<SocketAcceptor>{mm, std::move(addr)};
    result->register_at_parent(result);
    result->get_acceptor().init(std::move(fd));
    return result;
}

} // namespace actor_io
} // namespace boost

#endif // tcp_PEER_ACCEPTOR_HPP
