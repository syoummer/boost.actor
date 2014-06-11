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


#include "boost/actor/abstract_channel.hpp"
#include "boost/actor/detail/singletons.hpp"

namespace boost {
namespace actor {

using detail::singletons;

abstract_channel::abstract_channel() : m_node(singletons::get_node_id()) { }

abstract_channel::abstract_channel(node_id_ptr nid) : m_node(std::move(nid)) { }

abstract_channel::~abstract_channel() { }

bool abstract_channel::is_remote() const {
    return m_node != singletons::get_node_id();
}

} // namespace actor
} // namespace boost
