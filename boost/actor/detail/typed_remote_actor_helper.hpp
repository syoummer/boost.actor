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
