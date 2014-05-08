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


#ifndef BOOST_ACTOR_PUBLISH_HPP
#define BOOST_ACTOR_PUBLISH_HPP

#include <cstdint>

#include "boost/actor/actor.hpp"

#include "boost/actor/io/acceptor.hpp"
#include "boost/actor/io/ipv4_acceptor.hpp"

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
    publish(std::move(whom), io::ipv4_acceptor::create(port, addr));
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
                         io::ipv4_acceptor::create(port, addr));
}

} // namespace actor
} // namespace boost


#endif // BOOST_ACTOR_PUBLISH_HPP
