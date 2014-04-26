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
