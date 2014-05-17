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


#ifndef BOOST_ACTOR_FWD_HPP
#define BOOST_ACTOR_FWD_HPP

#include <cstdint>

namespace boost {

template<class> class intrusive_ptr;

namespace actor {

// classes
class actor;
class group;
class channel;
class node_id;
class behavior;
class resumable;
class message;
class actor_addr;
class actor_proxy;
class scoped_actor;
class execution_unit;
class abstract_actor;
class abstract_group;
class blocking_actor;
class message_header;
class message_handler;
class uniform_type_info;
class event_based_actor;

// structs
struct anything;

// enums
enum primitive_type : unsigned char;
enum class atom_value : std::uint64_t;

// intrusive pointer typedefs
typedef intrusive_ptr<abstract_group>      abstract_group_ptr;
typedef intrusive_ptr<actor_proxy>         actor_proxy_ptr;
typedef intrusive_ptr<node_id>             node_id_ptr;

// convenience typedefs
typedef const message_header& msg_hdr_cref;

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_FWD_HPP
