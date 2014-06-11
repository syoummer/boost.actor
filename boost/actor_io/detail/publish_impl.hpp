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
#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/actor_registry.hpp"

#include "boost/actor_io/network.hpp"
#include "boost/actor_io/middleman.hpp"
#include "boost/actor_io/basp_broker.hpp"

namespace boost {
namespace actor_io {
namespace detail {

template<class ActorHandle, class SocketAcceptor>
void publish_impl(ActorHandle whom, SocketAcceptor fd) {
    using namespace actor::detail;
    auto mm = middleman::instance();
    // we can't move fd into our lambda in C++11 ...
    using pair_type = std::pair<ActorHandle, SocketAcceptor>;
    auto data = std::make_shared<pair_type>(std::move(whom), std::move(fd));
    mm->run_later([mm, data] {
        auto bro = mm->get_named_broker<basp_broker>(actor::atom("_BASP"));
        bro->publish(std::move(data->first), std::move(data->second));
    });
}

inline void publish_impl(actor::abstract_actor_ptr whom,
                         uint16_t port,
                         const char* ipaddr) {
    auto mm = middleman::instance();
    auto addr = whom->address();
    network::default_socket_acceptor fd{mm->backend()};
    network::ipv4_bind(fd, port, ipaddr);
    publish_impl(std::move(whom), std::move(fd));
}

} // namespace detail
} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_PUBLISH_IMPL_HPP
