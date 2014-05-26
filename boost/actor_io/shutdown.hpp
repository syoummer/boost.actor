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


#ifndef BOOST_ACTOR_IO_SHUTDOWN_HPP
#define BOOST_ACTOR_IO_SHUTDOWN_HPP

namespace boost {
namespace actor_io {

// note: implemented in middleman.cpp

/**
 * @brief Destroys all singletons, disconnects all peers and stops the
 *        scheduler. It is recommended to use this function as very last
 *        function call before leaving main(). Especially in programs
 *        using libcppa's networking infrastructure.
 */
void shutdown();

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_IO_SHUTDOWN_HPP
