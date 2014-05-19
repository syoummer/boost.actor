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


#ifndef BOOST_ACTOR_REMOTE_ACTOR_HPP
#define BOOST_ACTOR_REMOTE_ACTOR_HPP

#include <set>
#include <string>
#include <cstdint>

#include "boost/actor/io/acceptor.hpp"
#include "boost/actor/io/tcp_io_stream.hpp"

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
    auto ptr = io::tcp_io_stream::connect_to(host, port);
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
