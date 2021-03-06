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


#include <sstream>
#include <stdlib.h>


#include "boost/actor/config.hpp"
#include "boost/actor/exception.hpp"

#ifdef BOOST_ACTOR_WINDOWS
#   include <winerror.h>
#else
#   include <errno.h>
#   include <sys/socket.h>
#   include <sys/un.h>
#endif

namespace {

std::string ae_what(std::uint32_t reason) {
    std::ostringstream oss;
    oss << "actor exited with reason " << reason;
    return oss.str();
}

std::string be_what(int err_code) {
    switch (err_code) {
#   ifndef BOOST_ACTOR_WINDOWS
        case EACCES: return "EACCES: address protected; root access needed";
        case EADDRINUSE: return "EADDRINUSE: address already in use";
        case EBADF: return "EBADF: no valid socket descriptor";
        case EINVAL: return "EINVAL: socket already bound to an address";
        case ENOTSOCK: return "ENOTSOCK: no file descriptor given";
#   else
        case WSAEACCES: return "EACCES: address protected; root access needed";
        case WSAEADDRINUSE: return "EADDRINUSE: address already in use";
        case WSAEBADF: return "EBADF: no valid socket descriptor";
        case WSAEINVAL: return "EINVAL: socket already bound to an address";
        case WSAENOTSOCK: return "ENOTSOCK: no file descriptor given";
#   endif
        default: break;
    }
    std::stringstream oss;
    oss << "an unknown error occurred (code: " << err_code << ")";
    return oss.str();
}

} // namespace <anonymous>


namespace boost {
namespace actor {

cppa_exception::~cppa_exception() { }

cppa_exception::cppa_exception(const std::string& what_str)
: m_what(what_str) { }

cppa_exception::cppa_exception(std::string&& what_str)
: m_what(std::move(what_str)) { }

const char* cppa_exception::what() const noexcept {
    return m_what.c_str();
}

actor_exited::~actor_exited() { }

actor_exited::actor_exited(std::uint32_t reason)
: cppa_exception(ae_what(reason)) {
    m_reason = reason;
}

network_error::~network_error() { }

network_error::network_error(const std::string& str) : cppa_exception(str) { }

network_error::network_error(std::string&& str)
: cppa_exception(std::move(str)) { }

bind_failure::bind_failure(int err_code) : network_error(be_what(err_code)) {
    m_errno = err_code;
}

bind_failure::~bind_failure() { }

} // namespace actor
} // namespace boost
