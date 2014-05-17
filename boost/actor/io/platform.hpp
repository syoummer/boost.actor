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










#ifndef BOOST_ACTOR_IO_PLATFORM_HPP
#define BOOST_ACTOR_IO_PLATFORM_HPP

#include "boost/config.hpp"

#ifdef BOOST_WINDOWS
#   include <w32api.h>
#   undef _WIN32_WINNT
#   undef WINVER
#   define _WIN32_WINNT WindowsVista
#   define WINVER WindowsVista
#   include <ws2tcpip.h>
#   include <winsock2.h>
    // remove interface which is defined in rpc.h in files included by
    // windows.h as it clashes with name used in own code
#   undef interface
#else
#   include <unistd.h>
#   include <errno.h>
#endif

namespace boost {
namespace actor {
namespace io {

// platform-dependent types for sockets and some utility functions
#ifdef BOOST_WINDOWS
    typedef SOCKET native_socket_type;
    typedef const char* setsockopt_ptr;
    typedef const char* socket_send_ptr;
    typedef char* socket_recv_ptr;
    typedef int socklen_t;
    constexpr SOCKET invalid_socket = INVALID_SOCKET;
    inline int last_socket_error() { return WSAGetLastError(); }
    inline bool would_block_or_temporarily_unavailable(int errcode) {
        return errcode == WSAEWOULDBLOCK || errcode == WSATRY_AGAIN;
    }
#else
    typedef int native_socket_type;
    typedef const void* setsockopt_ptr;
    typedef const void* socket_send_ptr;
    typedef void* socket_recv_ptr;
    constexpr int invalid_socket = -1;
    inline void closesocket(native_socket_type fd) { close(fd); }
    inline int last_socket_error() { return errno; }
    inline bool would_block_or_temporarily_unavailable(int errcode) {
        return errcode == EAGAIN || errcode == EWOULDBLOCK;
    }
#endif

} // namespace boost
} // namespace actor
} // namespace io

#endif // BOOST_ACTOR_IO_PLATFORM_HPP
