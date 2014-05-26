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


#ifndef BOOST_ACTOR_SPAWN_IO_HPP
#define BOOST_ACTOR_SPAWN_IO_HPP

#include "boost/asio.hpp"

#include "boost/actor/connection_handle.hpp"

#include "boost/actor_io/broker.hpp"

namespace boost {
namespace actor_io {

/**
 * @brief Spawns a new functor-based broker.
 */
template<actor::spawn_options Os = actor::no_spawn_options,
         typename F = std::function<void (broker*)>,
         typename... Ts>
actor::actor spawn_io(F fun, Ts&&... args) {
    auto ptr = broker::from(std::move(fun), std::forward<Ts>(args)...);
    return {init_and_launch(std::move(ptr))};
}

/**
 * @brief Spawns a new functor-based broker adapting
 *        given streams as connection.
 */
template<actor::spawn_options Os = actor::no_spawn_options,
         typename F = std::function<void (broker*)>,
         typename Socket = network::default_socket,
         typename... Ts>
typename std::enable_if<
    !std::is_convertible<Socket, std::string>::value,
    actor::actor
>::type
spawn_io_client(F fun, Socket fd, Ts&&... args) {
    auto hdl = actor::connection_handle::from_int(fd.native_handle());
    auto ptr = broker::from(std::move(fun), hdl, std::forward<Ts>(args)...);
    ptr->add_connection(std::move(fd));
    return {init_and_launch(std::move(ptr))};
}

/**
 * @brief Spawns a new functor-based broker connecting to <tt>host:port</tt>.
 */
template<actor::spawn_options Os = actor::no_spawn_options,
         typename F = std::function<void (broker*)>,
         typename... Ts>
actor::actor spawn_io_client(F fun, const std::string& host, uint16_t port, Ts&&... args) {
    network::default_socket fd{middleman::instance()->backend()};
    network::connect_ipv4_socket(fd, host, port);
    return spawn_io_client(std::move(fun), std::move(fd), std::forward<Ts>(args)...);
}

/**
 * @brief Spawns a new broker as server running on given @p port.
 */
template<actor::spawn_options Os = actor::no_spawn_options,
         typename F = std::function<void (broker*)>,
         typename Socket = network::default_socket_acceptor,
         typename... Ts>
actor::actor spawn_io_server(F fun, Socket fd, Ts&&... args) {
    auto ptr = broker::from(std::move(fun), std::forward<Ts>(args)...);
    ptr->add_acceptor(std::move(fd));
    return {init_and_launch(std::move(ptr))};
}

/**
 * @brief Spawns a new broker as server running on given @p port.
 */
template<actor::spawn_options Os = actor::no_spawn_options,
         typename F = std::function<void (broker*)>,
         typename... Ts>
actor::actor spawn_io_server(F fun, uint16_t port, Ts&&... args) {
    network::default_socket_acceptor fd{middleman::instance()->backend()};
    network::bind_ipv4_socket_acceptor(fd, port);
    return spawn_io_server(std::move(fun), std::move(fd), std::forward<Ts>(args)...);
    /*
    static_assert(!has_detach_flag(Os),
                  "brokers cannot be detached");
    static_assert(is_unbound(Os),
                  "top-level spawns cannot have monitor or link flag");
    using namespace std;
    auto aptr = tcp_acceptor::create(port);
    auto ptr = broker::from(move(fun), forward<Ts>(args)...);
    ptr->add_acceptor(std::move(aptr));
    return {init_and_launch(move(ptr))};
    */
}

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_SPAWN_IO_HPP
