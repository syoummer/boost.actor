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


#ifndef BOOST_ACTOR_DETAIL_TYPED_REMOTE_ACTOR_HELPER_HPP
#define BOOST_ACTOR_DETAIL_TYPED_REMOTE_ACTOR_HELPER_HPP

#include "boost/actor/typed_actor.hpp"

#include "boost/actor/io/acceptor.hpp"
#include "boost/actor/io/ipv4_io_stream.hpp"

#include "boost/actor/detail/type_list.hpp"
#include "boost/actor/detail/raw_access.hpp"

namespace boost {
namespace actor {
namespace detail {

template<class List>
struct typed_remote_actor_helper;

template<typename... Ts>
struct typed_remote_actor_helper<detail::type_list<Ts...>> {
    typedef typed_actor<Ts...> return_type;
    return_type operator()(io::stream_ptr_pair conn) {
        auto iface = return_type::get_interface();
        auto tmp = remote_actor_impl(std::move(conn), std::move(iface));
        return_type res;
        // actually safe, because remote_actor_impl throws on type mismatch
        raw_access::unsafe_assign(res, tmp);
        return res;
    }
    return_type operator()(const char* host, std::uint16_t port) {
        auto ptr = io::ipv4_io_stream::connect_to(host, port);
        return (*this)(io::stream_ptr_pair(ptr, ptr));
    }
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_TYPED_REMOTE_ACTOR_HELPER_HPP
