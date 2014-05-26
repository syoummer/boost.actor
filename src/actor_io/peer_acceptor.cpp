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


#include <cstring>
#include <iostream>
#include <exception>

#include "boost/actor/node_id.hpp"
#include "boost/actor/to_string.hpp"
#include "boost/actor/binary_serializer.hpp"

#include "boost/actor/detail/logging.hpp"
#include "boost/actor/detail/demangle.hpp"

#include "boost/actor_io/peer.hpp"
#include "boost/actor_io/middleman.hpp"
#include "boost/actor_io/peer_acceptor.hpp"

using namespace std;

namespace boost {
namespace actor_io {

peer_acceptor::peer_acceptor(middleman* parent, actor::actor_addr&& addr)
: m_parent(parent), m_aa(std::move(addr)) { }

peer_acceptor::~peer_acceptor() {
    // nop
}

void peer_acceptor::handle_io_failure(const network::manager_ptr& self,
                                      const std::string& error_description) {
    BOOST_ACTOR_LOG_INFO("removed peer_acceptor "
                         << this << " due to an IO failure: "
                         << error_description);
    static_cast<void>(error_description);
    parent()->remove(self);
}

network::multiplexer& peer_acceptor::backend() {
    return parent()->backend();
}

void peer_acceptor::register_at_parent(const network::manager_ptr& self) {
    parent()->add(self);
}

} // namespace actor_io
} // namespace boost
