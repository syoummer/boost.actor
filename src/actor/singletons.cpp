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


#include <atomic>
#include <iostream>

#include "boost/actor/message.hpp"
#include "boost/actor/scheduler.hpp"
#include "boost/actor/exception.hpp"
#include "boost/actor/local_actor.hpp"

#include "boost/actor/detail/logging.hpp"
#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/group_manager.hpp"
#include "boost/actor/detail/actor_registry.hpp"
#include "boost/actor/detail/uniform_type_info_map.hpp"

namespace boost {
namespace actor {
namespace detail {

namespace {

std::atomic<abstract_singleton*> s_plugins[singletons::max_plugin_singletons];
std::atomic<scheduler::coordinator*> s_scheduling_coordinator;
std::atomic<uniform_type_info_map*> s_uniform_type_info_map;
std::atomic<actor_registry*> s_actor_registry;
std::atomic<group_manager*> s_group_manager;
std::atomic<node_id::data*> s_node_id;
std::atomic<logging*> s_logger;

} // namespace <anonymous>

abstract_singleton::~abstract_singleton() {
    // nop
}

void singletons::stop_singletons() {
    BOOST_ACTOR_LOGF_DEBUG("destroy scheduler");
    destroy(s_scheduling_coordinator);
    BOOST_ACTOR_LOGF_DEBUG("destroy plugins");
    for (auto& plugin : s_plugins) destroy(plugin);
    BOOST_ACTOR_LOGF_DEBUG("destroy actor registry");
    destroy(s_actor_registry);
    BOOST_ACTOR_LOGF_DEBUG("destroy group manager");
    destroy(s_group_manager);
    BOOST_ACTOR_LOGF_DEBUG("destroy type info map");
    destroy(s_uniform_type_info_map);
    destroy(s_logger);
    destroy(s_node_id);
}

actor_registry* singletons::get_actor_registry() {
    return lazy_get(s_actor_registry);
}

uniform_type_info_map* singletons::get_uniform_type_info_map() {
    return lazy_get(s_uniform_type_info_map);
}

group_manager* singletons::get_group_manager() {
    return lazy_get(s_group_manager);
}

scheduler::coordinator* singletons::get_scheduling_coordinator() {
    return lazy_get(s_scheduling_coordinator);
}

node_id singletons::get_node_id() {
    return node_id{lazy_get(s_node_id)};
}

logging* singletons::get_logger() {
    return lazy_get(s_logger);
}

std::atomic<abstract_singleton*>& singletons::get_plugin_singleton(size_t id) {
    BOOST_ACTOR_REQUIRE(id < max_plugin_singletons);
    return s_plugins[id];
}

} // namespace detail
} // namespace actor
} // namespace boost
