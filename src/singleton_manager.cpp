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


#include <atomic>
#include <iostream>

#include "boost/actor/logging.hpp"
#include "boost/actor/scheduler.hpp"
#include "boost/actor/exception.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/local_actor.hpp"

#include "boost/actor/io/middleman.hpp"

#include "boost/actor/detail/group_manager.hpp"
#include "boost/actor/detail/actor_registry.hpp"
#include "boost/actor/detail/singleton_manager.hpp"
#include "boost/actor/detail/uniform_type_info_map.hpp"

namespace boost {
namespace actor {

void shutdown() { detail::singleton_manager::shutdown(); }

} // namespace actor
} // namespace boost

namespace boost {
namespace actor {
namespace detail {

namespace {

std::atomic<uniform_type_info_map*> s_uniform_type_info_map;
std::atomic<io::middleman*> s_middleman;
std::atomic<actor_registry*> s_actor_registry;
std::atomic<group_manager*> s_group_manager;
std::atomic<scheduler::coordinator*> s_scheduling_coordinator;
std::atomic<logging*> s_logger;

} // namespace <anonymous>

void singleton_manager::shutdown() {
    BOOST_ACTOR_LOGF_DEBUG("shutdown scheduler");
    destroy(s_scheduling_coordinator);
    BOOST_ACTOR_LOGF_DEBUG("shutdown middleman");
    destroy(s_middleman);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    // it's safe to delete all other singletons now
    BOOST_ACTOR_LOGF_DEBUG("close actor registry");
    destroy(s_actor_registry);
    BOOST_ACTOR_LOGF_DEBUG("shutdown group manager");
    destroy(s_group_manager);
    BOOST_ACTOR_LOGF_DEBUG("clear type info map");
    destroy(s_uniform_type_info_map);
    destroy(s_logger);
}

actor_registry* singleton_manager::get_actor_registry() {
    return lazy_get(s_actor_registry);
}

uniform_type_info_map* singleton_manager::get_uniform_type_info_map() {
    return lazy_get(s_uniform_type_info_map);
}

group_manager* singleton_manager::get_group_manager() {
    return lazy_get(s_group_manager);
}

scheduler::coordinator* singleton_manager::get_scheduling_coordinator() {
    return lazy_get(s_scheduling_coordinator);
}

logging* singleton_manager::get_logger() {
    return lazy_get(s_logger);
}

io::middleman* singleton_manager::get_middleman() {
    return lazy_get(s_middleman);
}

} } // namespace actor
} // namespace boost::detail
