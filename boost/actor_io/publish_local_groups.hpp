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


#ifndef BOOST_ACTOR_IO_PUBLISH_LOCAL_GROUPS_HPP
#define BOOST_ACTOR_IO_PUBLISH_LOCAL_GROUPS_HPP

#include <cstdint>

namespace boost {
namespace actor_io {

/**
 * @brief Makes *all* local groups accessible via network on address @p addr
 *        and @p port.
 * @throws bind_failure
 * @throws network_error
 */
void publish_local_groups(uint16_t port, const char* addr = nullptr);

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_IO_PUBLISH_LOCAL_GROUPS_HPP
