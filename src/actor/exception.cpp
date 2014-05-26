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

std::string ae_what(uint32_t reason) {
    std::ostringstream oss;
    oss << "actor exited with reason " << reason;
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

actor_exited::actor_exited(uint32_t reason)
: cppa_exception(ae_what(reason)) {
    m_reason = reason;
}

network_error::network_error(const std::string& str) : super(str) { }

network_error::network_error(std::string&& str) : super(std::move(str)) { }

network_error::~network_error() { }

bind_failure::bind_failure(const std::string& str) : super(str) { }

bind_failure::bind_failure(std::string&& str) : super(std::move(str)) { }

bind_failure::~bind_failure() { }

} // namespace actor
} // namespace boost
