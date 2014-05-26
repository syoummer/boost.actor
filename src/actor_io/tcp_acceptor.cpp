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


#include <ios>
#include <cstring>
#include <errno.h>
#include <iostream>

#include "boost/none.hpp"

#include "boost/actor/config.hpp"
#include "boost/actor/exception.hpp"

#include "boost/actor/detail/logging.hpp"

#include "boost/actor_io/stream.hpp"
#include "boost/actor_io/fd_util.hpp"
#include "boost/actor_io/tcp_acceptor.hpp"
#include "boost/actor_io/tcp_io_stream.hpp"

#ifdef BOOST_ACTOR_WINDOWS
#   include <winsock2.h>
#   include <ws2tcpip.h>
#else
#   include <netdb.h>
#   include <unistd.h>
#   include <arpa/inet.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#endif

namespace boost {
namespace actor_io {

using namespace boost::actor;
using namespace boost::actor_io::fd_util;

namespace {

struct socket_guard {

    bool m_released;
    native_socket_type m_socket;

 public:

    socket_guard(native_socket_type sfd) : m_released(false), m_socket(sfd) { }

    ~socket_guard() {
        if (!m_released) closesocket(m_socket);
    }

    void release() {
        m_released = true;
    }

};

bool accept_impl(stream_ptr_pair& result,
                 native_socket_type fd,
                 bool nonblocking) {
    sockaddr addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrlen = sizeof(addr);
    auto sfd = ::accept(fd, &addr, &addrlen);
    if (sfd == invalid_socket) {
        auto err = last_socket_error();
        if (nonblocking && would_block_or_temporarily_unavailable(err)) {
            // ok, try again
            return false;
        }
        throw_io_failure("accept failed");
    }
    stream_ptr ptr(tcp_io_stream::from_native_socket(sfd));
    result.first = ptr;
    result.second = ptr;
    return true;
}

} // namespace <anonymous>

tcp_acceptor::tcp_acceptor(native_socket_type fd, bool nonblocking)
: m_fd(fd), m_is_nonblocking(nonblocking) { }

std::unique_ptr<acceptor> tcp_acceptor::create(std::uint16_t port,
                                                const char* addr) {
    BOOST_ACTOR_LOGM_TRACE("tcp_acceptor", BOOST_ACTOR_ARG(port) << ", addr = "
                                     << (addr ? addr : "nullptr"));
#   ifdef BOOST_ACTOR_WINDOWS
    // ensure that TCP has been initialized via WSAStartup
    cppa::get_middleman();
#   endif
    native_socket_type sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == invalid_socket) {
        throw network_error("could not create server socket");
    }
    // sguard closes the socket in case of exception
    socket_guard sguard(sockfd);
    int on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<setsockopt_ptr>(&on), sizeof(on)) < 0) {
        throw_io_failure("unable to set SO_REUSEADDR");
    }
    struct sockaddr_in serv_addr;
    memset((char*) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    if (! addr) {
        serv_addr.sin_addr.s_addr = INADDR_ANY;
    }
    else if (::inet_pton(AF_INET, addr, &serv_addr.sin_addr) <= 0) {
        throw network_error("invalid IPv4 address");
    }
    serv_addr.sin_port = htons(port);
    if (bind(sockfd, (sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        throw bind_failure(errno);
    }
    if (listen(sockfd, 10) != 0) {
        throw network_error("listen() failed");
    }
    // default mode is nonblocking
    nonblocking(sockfd, true);
    // ok, no exceptions
    sguard.release();
    BOOST_ACTOR_LOGM_DEBUG("tcp_acceptor", "sockfd = " << sockfd);
    return std::unique_ptr<tcp_acceptor>(new tcp_acceptor(sockfd, true));
}

tcp_acceptor::~tcp_acceptor() {
    closesocket(m_fd);
}

native_socket_type tcp_acceptor::file_handle() const {
    return m_fd;
}

stream_ptr_pair tcp_acceptor::accept_connection() {
    if (m_is_nonblocking) {
        nonblocking(m_fd, false);
        m_is_nonblocking = false;
    }
    stream_ptr_pair result;
    accept_impl(result, m_fd, m_is_nonblocking);
    return result;
}

optional<stream_ptr_pair> tcp_acceptor::try_accept_connection() {
    if (!m_is_nonblocking) {
        nonblocking(m_fd, true);
        m_is_nonblocking = true;
    }
    stream_ptr_pair result;
    if (accept_impl(result, m_fd, m_is_nonblocking)) {
        return result;
    }
    return none;
}

} // namespace actor_io
} // namespace boost
