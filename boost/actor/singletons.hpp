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


#ifndef BOOST_ACTOR_SINGLETONS_HPP
#define BOOST_ACTOR_SINGLETONS_HPP

#include "boost/actor/detail/singleton_manager.hpp"

namespace boost {
namespace actor {

inline logging* get_logger() {
    return detail::singleton_manager::get_logger();
}

inline scheduler::coordinator* get_scheduling_coordinator() {
    return detail::singleton_manager::get_scheduling_coordinator();
}

inline detail::group_manager* get_group_manager() {
    return detail::singleton_manager::get_group_manager();
}

inline detail::actor_registry* get_actor_registry() {
    return detail::singleton_manager::get_actor_registry();
}

inline io::middleman* get_middleman() {
    return detail::singleton_manager::get_middleman();
}

inline detail::uniform_type_info_map* get_uniform_type_info_map() {
    return detail::singleton_manager::get_uniform_type_info_map();
}

inline detail::message_data* get_tuple_dummy() {
    return detail::singleton_manager::get_tuple_dummy();
}

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_SINGLETONS_HPP
