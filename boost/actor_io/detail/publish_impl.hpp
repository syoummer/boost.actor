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


#ifndef BOOST_ACTOR_DETAIL_PUBLISH_IMPL_HPP
#define BOOST_ACTOR_DETAIL_PUBLISH_IMPL_HPP

#include "boost/asio.hpp"

#include "boost/actor/abstract_actor.hpp"
#include "boost/actor/detail/raw_access.hpp"
#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/actor_registry.hpp"

#include "boost/actor_io/network.hpp"
#include "boost/actor_io/middleman.hpp"
#include "boost/actor_io/peer_acceptor.hpp"

#include "boost/actor_io/network.hpp"

namespace boost {
namespace actor_io {
namespace detail {

template<class AcceptSocket>
void publish_impl(actor::abstract_actor_ptr whom,
                  peer_acceptor::impl<AcceptSocket>* ptr) {
    // behind the scenes, we serialze/deserialize as actor
    BOOST_ACTOR_LOGF_TRACE(BOOST_ACTOR_TARG(whom, to_string));
    actor::detail::singletons::get_actor_registry()
    ->put(whom->id(), whom.get());
    ptr->start();
}

template<class Protocol>
void publish_impl(actor::abstract_actor_ptr whom,
                  asio::basic_socket_acceptor<Protocol> fd) {
    using namespace boost::actor::detail;
    typedef asio::basic_stream_socket<Protocol> sock_type;
    auto mm = middleman::instance();
    auto addr = whom->address();
    auto sigs = whom->interface();
    auto ptr = peer_acceptor::make(mm, std::move(addr),
                                   std::move(sigs), std::move(fd));
    publish_impl(std::move(whom), ptr);
}

inline void publish_impl(boost::actor::abstract_actor_ptr whom,
                         uint16_t port,
                         const char* ipaddr) {
    auto mm = middleman::instance();
    auto addr = whom->address();
    network::default_socket_acceptor fd{mm->backend()};
    auto ptr = peer_acceptor::make(mm, std::move(addr), std::move(fd));
    network::bind_ipv4_acceptor(ptr->get_acceptor(), port, ipaddr);
    publish_impl(std::move(whom), ptr);
}

} // namespace detail
} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_PUBLISH_IMPL_HPP
