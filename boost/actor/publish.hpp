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


#ifndef BOOST_ACTOR_PUBLISH_HPP
#define BOOST_ACTOR_PUBLISH_HPP

#include <cstdint>

#include "boost/actor/actor.hpp"

#include "boost/actor/io/acceptor.hpp"
#include "boost/actor/io/tcp_acceptor.hpp"

#include "boost/actor/detail/raw_access.hpp"
#include "boost/actor/detail/publish_impl.hpp"

namespace boost {
namespace actor {

/**
 * @brief Publishes @p whom using @p acceptor to handle incoming connections.
 *
 * The connection is automatically closed if the lifetime of @p whom ends.
 * @param whom Actor that should be published at @p port.
 * @param acceptor Network technology-specific acceptor implementation.
 */
inline void publish(actor whom, io::acceptor_uptr acceptor) {
    detail::publish_impl(detail::raw_access::get(whom), std::move(acceptor));
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
inline void publish(actor whom, uint16_t port, const char* addr = nullptr) {
    if (!whom) return;
    publish(std::move(whom), io::tcp_acceptor::create(port, addr));
}

/**
 * @copydoc publish(actor,io::acceptor_uptr)
 */
template<typename... Rs>
void typed_publish(typed_actor<Rs...> whom, io::acceptor_uptr uptr) {
    if (!whom) return;
    detail::publish_impl(detail::raw_access::get(whom), std::move(uptr));
}

/**
 * @copydoc publish(actor,std::uint16_t,const char*)
 */
template<typename... Rs>
void typed_publish(typed_actor<Rs...> whom,
                   std::uint16_t port,
                   const char* addr = nullptr) {
    if (!whom) return;
    detail::publish_impl(detail::raw_access::get(whom),
                         io::tcp_acceptor::create(port, addr));
}

} // namespace actor
} // namespace boost


#endif // BOOST_ACTOR_PUBLISH_HPP
