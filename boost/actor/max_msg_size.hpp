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


#ifndef BOOST_ACTOR_MAX_MSG_SIZE_HPP
#define BOOST_ACTOR_MAX_MSG_SIZE_HPP

#include <cstddef> // size_t

namespace boost {
namespace actor {

/**
 * @brief Sets the maximum size of a message over network.
 * @param size The maximum number of bytes a message may occupy.
 */
void max_msg_size(size_t size);

/**
 * @brief Queries the maximum size of messages over network.
 * @returns The number maximum number of bytes a message may occupy.
 */
size_t max_msg_size();

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_MAX_MSG_SIZE_HPP
