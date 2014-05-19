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


#ifndef BOOST_ACTOR_tcp_IO_STREAM_HPP
#define BOOST_ACTOR_tcp_IO_STREAM_HPP

#include "boost/actor/io/stream.hpp"
#include "boost/actor/io/platform.hpp"

namespace boost {
namespace actor {
namespace io {

class tcp_io_stream : public stream {

 public:

    ~tcp_io_stream();

    static stream_ptr connect_to(const char* host, std::uint16_t port);

    static stream_ptr from_native_socket(native_socket_type fd);

    native_socket_type read_handle() const;

    native_socket_type write_handle() const;

    void read(void* buf, size_t len);

    size_t read_some(void* buf, size_t len);

    void write(const void* buf, size_t len);

    size_t write_some(const void* buf, size_t len);

    static stream_ptr from_sockfd(native_socket_type fd);

 private:

    tcp_io_stream(native_socket_type fd);

    native_socket_type m_fd;

};

} // namespace io
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_tcp_IO_STREAM_HPP
