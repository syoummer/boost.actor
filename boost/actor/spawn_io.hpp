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


#ifndef BOOST_ACTOR_SPAWN_IO_HPP
#define BOOST_ACTOR_SPAWN_IO_HPP

#include "boost/actor/io/broker.hpp"
#include "boost/actor/io/ipv4_io_stream.hpp"

namespace boost {
namespace actor {

/**
 * @brief Spawns a new functor-based broker.
 */
template<spawn_options Os = no_spawn_options,
         typename F = std::function<void (io::broker*)>,
         typename... Ts>
actor spawn_io(F fun, Ts&&... args) {
    auto ptr = io::broker::from(std::move(fun), std::forward<Ts>(args)...);
    return {io::init_and_launch(std::move(ptr))};
}

/**
 * @brief Spawns a new functor-based broker adapting
 *        given streams as connection.
 */
template<spawn_options Os = no_spawn_options,
         typename F = std::function<void (io::broker*)>,
         typename... Ts>
actor spawn_io_client(F fun,
                      io::input_stream_ptr in,
                      io::output_stream_ptr out,
                      Ts&&... args) {
    auto hdl = io::connection_handle::from_int(in->read_handle());
    auto ptr = io::broker::from(std::move(fun), hdl, std::forward<Ts>(args)...);
    ptr->add_connection(std::move(in), std::move(out));
    return {io::init_and_launch(std::move(ptr))};
}

/**
 * @brief Spawns a new functor-based broker connecting to <tt>host:port</tt>.
 */
template<spawn_options Os = no_spawn_options,
         typename F = std::function<void (io::broker*)>,
         typename... Ts>
actor spawn_io_client(F fun, const std::string& host, uint16_t port, Ts&&... args) {
    auto ptr = io::ipv4_io_stream::connect_to(host.c_str(), port);
    return spawn_io_client(std::move(fun), ptr, ptr, std::forward<Ts>(args)...);
}

/**
 * @brief Spawns a new broker as server running on given @p port.
 */
template<spawn_options Os = no_spawn_options,
         typename F = std::function<void (io::broker*)>,
         typename... Ts>
actor spawn_io_server(F fun, uint16_t port, Ts&&... args) {
    static_assert(!has_detach_flag(Os),
                  "brokers cannot be detached");
    static_assert(is_unbound(Os),
                  "top-level spawns cannot have monitor or link flag");
    using namespace std;
    auto aptr = io::ipv4_acceptor::create(port);
    auto ptr = io::broker::from(move(fun), forward<Ts>(args)...);
    ptr->add_acceptor(std::move(aptr));
    return {io::init_and_launch(move(ptr))};
}

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_SPAWN_IO_HPP
