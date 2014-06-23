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


#ifndef BOOST_ACTOR_DETAIL_REMOTE_ACTOR_IMPL_HPP
#define BOOST_ACTOR_DETAIL_REMOTE_ACTOR_IMPL_HPP

#include <set>
#include <ios>
#include <string>
#include <vector>
#include <future>
#include <cstdint>
#include <algorithm>

#include "boost/asio.hpp"
#include "boost/system/error_code.hpp"

#include "boost/actor/abstract_actor.hpp"
#include "boost/actor/binary_deserializer.hpp"

#include "boost/actor/detail/logging.hpp"

#include "boost/actor_io/network.hpp"
#include "boost/actor_io/middleman.hpp"
#include "boost/actor_io/basp_broker.hpp"

namespace boost {
namespace actor_io {
namespace detail {

constexpr uint32_t max_iface_size = 100;

constexpr uint32_t max_iface_clause_size = 500;

template<class Socket>
actor::abstract_actor_ptr remote_actor_impl(Socket fd,
                                            const std::set<std::string>& ifs) {
    auto mm = middleman::instance();
    std::string error_msg;
    std::promise<actor::abstract_actor_ptr> result_promise;
    // we can't move fd into our lambda in C++11 ...
    auto fd_ptr = std::make_shared<Socket>(std::move(fd));
    basp_broker::client_handshake_data hdata{actor::invalid_node_id,
                                             &result_promise, &error_msg, &ifs};
    auto hdata_ptr = &hdata;
    mm->run_later([=] {
        auto bro = mm->get_named_broker<basp_broker>(actor::atom("_BASP"));
        auto hdl = bro->add_connection(std::move(*fd_ptr));
        bro->init_client(hdl, hdata_ptr);
    });
    auto result = result_promise.get_future().get();
    if (!result) throw std::runtime_error(error_msg);
    return result;
}

inline actor::abstract_actor_ptr remote_actor_impl(const std::string& host,
                                                   uint16_t port,
                                                   const std::set<std::string>& ifs) {
    auto mm = middleman::instance();
    network::default_socket fd{mm->backend()};
    network::ipv4_connect(fd, host, port);
    return remote_actor_impl(std::move(fd), ifs);
}

} // namespace detail
} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_REMOTE_ACTOR_IMPL_HPP
