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


#ifndef BOOST_ACTOR_DETAIL_PUBLISH_IMPL_HPP
#define BOOST_ACTOR_DETAIL_PUBLISH_IMPL_HPP

#include "boost/actor/abstract_actor.hpp"

#include "boost/actor_io/acceptor.hpp"

namespace boost {
namespace actor_io {
namespace detail {

// functions are implemented in unicast_network.cpp

void publish_impl(actor::abstract_actor_ptr whom, acceptor_uptr aptr);

} // namespace detail
} // namespace actor_io
} // namespace boost


#endif // BOOST_ACTOR_DETAIL_PUBLISH_IMPL_HPP
