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


#ifndef BOOST_ACTOR_REMOTE_ACTOR_HPP
#define BOOST_ACTOR_REMOTE_ACTOR_HPP

#include <set>
#include <string>
#include <cstdint>

#include "boost/actor/io/acceptor.hpp"
#include "boost/actor/io/ipv4_io_stream.hpp"

#include "boost/actor/detail/raw_access.hpp"
#include "boost/actor/detail/remote_actor_impl.hpp"
#include "boost/actor/detail/typed_remote_actor_helper.hpp"

namespace boost {
namespace actor {

/**
 * @brief Establish a new connection to a remote actor via @p connection.
 * @param connection A connection to another libcppa process described by a pair
 *                   of input and output stream.
 * @returns An {@link actor_ptr} to the proxy instance
 *          representing a remote actor.
 * @throws std::invalid_argument Thrown when connecting to a typed actor.
 */
inline actor remote_actor(io::stream_ptr_pair connection) {
    auto res = detail::remote_actor_impl(connection, std::set<std::string>{});
    return detail::raw_access::unsafe_cast(res);
}
/**
 * @brief Establish a new connection to the actor at @p host on given @p port.
 * @param host Valid hostname or IP address.
 * @param port TCP port.
 * @returns An {@link actor_ptr} to the proxy instance
 *          representing a remote actor.
 * @throws std::invalid_argument Thrown when connecting to a typed actor.
 */
inline actor remote_actor(const char* host, uint16_t port) {
    auto ptr = io::ipv4_io_stream::connect_to(host, port);
    return remote_actor(io::stream_ptr_pair{ptr, ptr});
}

/**
 * @copydoc remote_actor(const char*, std::uint16_t)
 */
inline actor remote_actor(const std::string& host, uint16_t port) {
    return remote_actor(host.c_str(), port);
}

/**
 * @copydoc remote_actor(io::stream_ptr_pair)
 */
template<class List>
typename detail::typed_remote_actor_helper<List>::return_type
typed_remote_actor(io::stream_ptr_pair connection) {
    detail::typed_remote_actor_helper<List> f;
    return f(std::move(connection));
}

/**
 * @copydoc remote_actor(const char*,std::uint16_t)
 */
template<class List>
typename detail::typed_remote_actor_helper<List>::return_type
typed_remote_actor(const char* host, std::uint16_t port) {
    detail::typed_remote_actor_helper<List> f;
    return f(host, port);
}

/**
 * @copydoc remote_actor(const std::string&,std::uint16_t)
 */
template<class List>
typename detail::typed_remote_actor_helper<List>::return_type
typed_remote_actor(const std::string& host, std::uint16_t port) {
    detail::typed_remote_actor_helper<List> f;
    return f(host.c_str(), port);
}

} // namespace actor
} // namespace boost


#endif // BOOST_ACTOR_REMOTE_ACTOR_HPP
