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


#ifndef BOOST_ACTOR_tcp_ACCEPTOR_HPP
#define BOOST_ACTOR_tcp_ACCEPTOR_HPP

#include <memory>
#include <cstdint>

#include "boost/actor_io/platform.hpp"
#include "boost/actor_io/acceptor.hpp"

namespace boost {
namespace actor_io {

class tcp_acceptor : public acceptor {

 public:

    static std::unique_ptr<acceptor> create(std::uint16_t port,
                                            const char* addr = nullptr);

    static std::unique_ptr<acceptor> from_sockfd(native_socket_type fd);

    ~tcp_acceptor();

    native_socket_type file_handle() const;

    stream_ptr_pair accept_connection();

    optional<stream_ptr_pair> try_accept_connection();

 private:

    tcp_acceptor(native_socket_type fd, bool nonblocking);

    native_socket_type m_fd;
    bool m_is_nonblocking;

};

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_tcp_ACCEPTOR_HPP
