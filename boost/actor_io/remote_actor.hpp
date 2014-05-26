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


#ifndef BOOST_ACTOR_REMOTE_ACTOR_HPP
#define BOOST_ACTOR_REMOTE_ACTOR_HPP

#include <set>
#include <string>
#include <cstdint>

#include "boost/actor/detail/raw_access.hpp"

#include "boost/actor_io/middleman.hpp"

#include "boost/actor_io/detail/remote_actor_impl.hpp"
#include "boost/actor_io/detail/typed_remote_actor_helper.hpp"

namespace boost {
namespace actor_io {

/**
 * @brief Establish a new connection to a remote actor via @p connection.
 * @param connection A connection to another process described by a pair
 *                   of input and output stream.
 * @returns An {@link actor_ptr} to the proxy instance
 *          representing a remote actor.
 * @throws std::invalid_argument Thrown when connecting to a typed actor.
 */
template<class Socket>
inline boost::actor::actor remote_actor(Socket fd) {
    auto res = detail::remote_actor_impl(std::move(fd), std::set<std::string>{});
    return actor::detail::raw_access::unsafe_cast(res);
}
/**
 * @brief Establish a new connection to the actor at @p host on given @p port.
 * @param host Valid hostname or IP address.
 * @param port TCP port.
 * @returns An {@link actor_ptr} to the proxy instance
 *          representing a remote actor.
 * @throws std::invalid_argument Thrown when connecting to a typed actor.
 */
inline boost::actor::actor remote_actor(const char* host, uint16_t port) {
    auto res = detail::remote_actor_impl(host, port, std::set<std::string>{});
    return actor::detail::raw_access::unsafe_cast(res);
}

/**
 * @copydoc remote_actor(const char*, uint16_t)
 */
inline boost::actor::actor remote_actor(const std::string& host, uint16_t port) {
    auto res = detail::remote_actor_impl(host, port, std::set<std::string>{});
    return actor::detail::raw_access::unsafe_cast(res);
}

/**
 * @copydoc remote_actor(stream_ptr_pair)
 */
template<class List, class Socket>
typename detail::typed_remote_actor_helper<List>::return_type
typed_remote_actor(Socket fd) {
    detail::typed_remote_actor_helper<List> f;
    return f(std::move(fd));
}

/**
 * @copydoc remote_actor(const char*,uint16_t)
 */
template<class List>
typename detail::typed_remote_actor_helper<List>::return_type
typed_remote_actor(const char* host, uint16_t port) {
    detail::typed_remote_actor_helper<List> f;
    return f(host, port);
}

/**
 * @copydoc remote_actor(const std::string&,uint16_t)
 */
template<class List>
typename detail::typed_remote_actor_helper<List>::return_type
typed_remote_actor(const std::string& host, uint16_t port) {
    detail::typed_remote_actor_helper<List> f;
    return f(host, port);
}

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_REMOTE_ACTOR_HPP
