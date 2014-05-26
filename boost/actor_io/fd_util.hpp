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


#ifndef FD_UTIL_HPP
#define FD_UTIL_HPP

#include <string>
#include <utility>  // std::pair
#include <unistd.h> // ssize_t

#include "boost/actor_io/platform.hpp"

namespace boost {
namespace actor_io {
namespace fd_util {

std::string last_socket_error_as_string();

// throws ios_base::failure and adds errno failure if @p add_errno_failure
void throw_io_failure [[noreturn]] (const char* what, bool add_errno = true);

// sets fd to nonblocking if <tt>set_nonblocking == true</tt>
// or to blocking if <tt>set_nonblocking == false</tt>
// throws @p ios_base::failure on error
void nonblocking(native_socket_type fd, bool new_value);

// returns true if fd is nodelay socket
// throws @p ios_base::failure on error
void tcp_nodelay(native_socket_type fd, bool new_value);

// reads @p result and @p errno and throws @p ios_base::failure on error
void handle_write_result(ssize_t result, bool is_nonblocking_io);

// reads @p result and @p errno and throws @p ios_base::failure on error
void handle_read_result(ssize_t result, bool is_nonblocking_io);

std::pair<native_socket_type, native_socket_type> create_pipe();

} // namespace fd_util
} // namespace actor_io
} // namespace boost

#endif // FD_UTIL_HPP
