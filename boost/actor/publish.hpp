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


#ifndef BOOST_ACTOR_PUBLISH_HPP
#define BOOST_ACTOR_PUBLISH_HPP

#include <cstdint>

#include "boost/asio.hpp"

#include "boost/actor/actor.hpp"

#include "boost/actor/detail/raw_access.hpp"

#include "boost/actor_io/detail/publish_impl.hpp"

namespace boost {
namespace actor_io {

/**
 * @brief Publishes @p whom using @p acceptor to handle incoming connections.
 *
 * The connection is automatically closed if the lifetime of @p whom ends.
 * @param whom Actor that should be published at @p port.
 * @param acceptor Network technology-specific acceptor implementation.
 */
template<class Protocol>
inline void publish(boost::actor::actor whom,
                    asio::basic_socket_acceptor<Protocol> acceptor) {
    typedef asio::basic_stream_socket<Protocol> sock_type;
    detail::publish_impl<sock_type>(boost::actor::detail::raw_access::get(whom),
                                    std::move(acceptor));
}

/**
 * @brief Publishes @p whom at @p port.
 *
 * The connection is automatically closed if the lifetime of @p whom ends.
 * @param whom Actor that should be published at @p port.
 * @param port Unused TCP port.
 * @param addr The IP address to listen to, or @p INADDR_ANY if @p addr is
 *             @p nullptr.
 * @throws bind_failure
 */
inline void publish(boost::actor::actor whom, uint16_t port, const char* addr = nullptr) {
    if (!whom) return;
    detail::publish_impl(boost::actor::detail::raw_access::get(whom), port, addr);
}

/**
 * @copydoc publish(actor,acceptor_uptr)
 */
template<class Protocol, typename... Rs>
void typed_publish(boost::actor::typed_actor<Rs...> whom,
                   asio::basic_socket_acceptor<Protocol> acceptor) {
    if (!whom) return;
    detail::publish_impl(boost::actor::detail::raw_access::get(whom),
                         std::move(acceptor));
}

/**
 * @copydoc publish(actor,uint16_t,const char*)
 */
template<typename... Rs>
void typed_publish(actor::typed_actor<Rs...> whom,
                   uint16_t port,
                   const char* addr = nullptr) {
    if (!whom) return;
    detail::publish_impl(actor::detail::raw_access::get(whom), port, addr);
}

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_PUBLISH_HPP
