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


#ifndef BOOST_ACTOR_EXIT_REASON_HPP
#define BOOST_ACTOR_EXIT_REASON_HPP

#include <cstdint>

namespace boost {
namespace actor { namespace exit_reason {

/**
 * @brief Indicates that the actor is still alive.
 */
static constexpr std::uint32_t not_exited = 0x00000;

/**
 * @brief Indicates that an actor finished execution.
 */
static constexpr std::uint32_t normal = 0x00001;

/**
 * @brief Indicates that an actor finished execution
 *        because of an unhandled exception.
 */
static constexpr std::uint32_t unhandled_exception = 0x00002;

/**
 * @brief Indicates that an event-based actor
 *        tried to use {@link receive()} or a strongly typed actor tried
 *        to call {@link become()}.
 */
static constexpr std::uint32_t unallowed_function_call = 0x00003;

/**
 * @brief Indicates that the actor received an unexpected
 *        synchronous reply message.
 */
static constexpr std::uint32_t unhandled_sync_failure = 0x00004;

/**
 * @brief Indicates that a synchronous message timed out.
 */
static constexpr std::uint32_t unhandled_sync_timeout = 0x00005;

/**
 * @brief Indicates that the actor was forced to shutdown by
 *        a user-generated event.
 */
static constexpr std::uint32_t user_shutdown = 0x00010;

/**
 * @brief Indicates that an actor finishied execution
 *        because a connection to a remote link was
 *        closed unexpectedly.
 */
static constexpr std::uint32_t remote_link_unreachable = 0x00101;

/**
 * @brief Any user defined exit reason should have a
 *        value greater or equal to prevent collisions
 *        with default defined exit reasons.
 */
static constexpr std::uint32_t user_defined = 0x10000;

const char* as_string(std::uint32_t value);

} } // namespace actor
} // namespace boost::exit_reason

#endif // BOOST_ACTOR_EXIT_REASON_HPP
