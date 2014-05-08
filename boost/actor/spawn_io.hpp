/******************************************************************************\
 *           ___        __                                                    *
 *          /\_ \    __/\ \                                                   *
 *          \//\ \  /\_\ \ \____    ___   _____   _____      __               *
 *            \ \ \ \/\ \ \ '__`\  /'___\/\ '__`\/\ '__`\  /'__`\             *
 *             \_\ \_\ \ \ \ \L\ \/\ \__/\ \ \L\ \ \ \L\ \/\ \L\.\_           *
 *             /\____\\ \_\ \_,__/\ \____\\ \ ,__/\ \ ,__/\ \__/.\_\          *
 *             \/____/ \/_/\/___/  \/____/ \ \ \/  \ \ \/  \/__/\/_/          *
 *                                          \ \_\   \ \_\                     *
 *                                           \/_/    \/_/                     *
 *                                                                            *
 * Copyright (C) 2011-2013                                                    *
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * This file is part of libcppa.                                              *
 * libcppa is free software: you can redistribute it and/or modify it under   *
 * the terms of the GNU Lesser General Public License as published by the     *
 * Free Software Foundation; either version 2.1 of the License,               *
 * or (at your option) any later version.                                     *
 *                                                                            *
 * libcppa is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with libcppa. If not, see <http://www.gnu.org/licenses/>.            *
\******************************************************************************/


#ifndef BOOST_ACTOR_SPAWN_IO_HPP
#define BOOST_ACTOR_SPAWN_IO_HPP

#include "boost/actor/io/broker.hpp"
#include "boost/actor/io/ipv4_io_stream.hpp"

namespace boost {
namespace actor {

/**
 * @brief Spawns a new, unconnected broker of type @p Impl.
 */
template<class Impl, spawn_options Os = no_spawn_options, typename... Ts>
actor spawn_io(Ts&&... args) {
    auto ptr = detail::make_counted<Impl>(std::forward<Ts>(args)...);
    return {io::init_and_launch(std::move(ptr))};
}

/**
 * @brief Spawns a new, unconnected broker from given functor.
 */
template<spawn_options Os = no_spawn_options,
         typename F = std::function<void (io::broker*)>,
         typename... Ts>
actor spawn_io(F fun, const std::string& host, uint16_t port, Ts&&... args) {
    auto ptr = io::ipv4_io_stream::connect_to(host.c_str(), port);
    return spawn_io(std::move(fun), ptr, ptr, std::forward<Ts>(args)...);
}

/**
 * @brief Spawns a new functor-based broker managing a connection
 *        given as input and output stream pointer.
 */
template<spawn_options Os = no_spawn_options,
         typename F = std::function<void (io::broker*)>,
         typename... Ts>
actor spawn_io(F fun,
               io::input_stream_ptr in,
               io::output_stream_ptr out,
               Ts&&... args) {
    auto ptr = io::broker::from(std::move(fun), std::move(in), std::move(out),
                                std::forward<Ts>(args)...);
    return {io::init_and_launch(std::move(ptr))};
}

/**
 * @brief Spawns a new functor-based broker managing acting as
 *        server at given port.
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
    auto ptr = io::broker::from(move(fun),
                                io::ipv4_acceptor::create(port),
                                forward<Ts>(args)...);
    return {io::init_and_launch(move(ptr))};
}

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_SPAWN_IO_HPP
