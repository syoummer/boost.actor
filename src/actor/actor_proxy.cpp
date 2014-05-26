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


#include <utility>
#include <iostream>

#include "boost/actor/atom.hpp"
#include "boost/actor/to_string.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/scheduler.hpp"
#include "boost/actor/actor_proxy.hpp"
#include "boost/actor/exit_reason.hpp"

#include "boost/actor_io/middleman.hpp"

#include "boost/actor/detail/singletons.hpp"

using namespace std;

namespace boost {
namespace actor {

actor_proxy::~actor_proxy() { }

actor_proxy::actor_proxy(actor_id mid) : super(mid) {
    m_node = detail::singletons::get_node_id();
}

} // namespace actor
} // namespace boost
