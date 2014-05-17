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


#ifndef BOOST_ACTOR_SYSTEM_MESSAGES_HPP
#define BOOST_ACTOR_SYSTEM_MESSAGES_HPP

#include <cstdint>
#include <type_traits>

#include "boost/actor/group.hpp"
#include "boost/actor/actor_addr.hpp"

#include "boost/actor/io/buffer.hpp"
#include "boost/actor/io/accept_handle.hpp"
#include "boost/actor/io/connection_handle.hpp"

#include "boost/actor/detail/tbind.hpp"
#include "boost/actor/detail/type_list.hpp"

namespace boost {
namespace actor {

/**
 * @brief Sent to all links when an actor is terminated.
 * @note This message can be handled manually by calling
 *       {@link local_actor::trap_exit(true) local_actor::trap_exit(bool)}
 *       and is otherwise handled implicitly by the runtime system.
 */
struct exit_msg {
    /**
     * @brief The source of this message, i.e., the terminated actor.
     */
    actor_addr source;
    /**
     * @brief The exit reason of the terminated actor.
     */
    std::uint32_t reason;
};

/**
 * @brief Sent to all actors monitoring an actor when it is terminated.
 */
struct down_msg {
    /**
     * @brief The source of this message, i.e., the terminated actor.
     */
    actor_addr source;
    /**
     * @brief The exit reason of the terminated actor.
     */
    std::uint32_t reason;
};

/**
 * @brief Sent to all members of a group when it goes offline.
 */
struct group_down_msg {
    /**
     * @brief The source of this message, i.e., the now unreachable group.
     */
    group source;
};

inline bool operator==(const group_down_msg& lhs, const group_down_msg& rhs) {
    return lhs.source == rhs.source;
}

inline bool operator!=(const group_down_msg& lhs, const group_down_msg& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Sent whenever a timeout occurs during a synchronous send.
 *
 * This system message does not have any fields, because the message ID
 * sent alongside this message identifies the matching request that timed out.
 */
struct sync_timeout_msg { };

/**
 * @relates exit_msg
 */
inline bool operator==(const sync_timeout_msg&, const sync_timeout_msg&) {
    return true;
}

/**
 * @relates exit_msg
 */
inline bool operator!=(const sync_timeout_msg&, const sync_timeout_msg&) {
    return false;
}

/**
 * @brief Sent whenever a terminated actor receives a synchronous request.
 */
struct sync_exited_msg {
    /**
     * @brief The source of this message, i.e., the terminated actor.
     */
    actor_addr source;
    /**
     * @brief The exit reason of the terminated actor.
     */
    std::uint32_t reason;
};

/**
 * @brief Signalizes a timeout event.
 * @note This message is handled implicitly by the runtime system.
 */
struct timeout_msg {
    /**
     * @brief Actor-specific timeout ID.
     */
    std::uint32_t timeout_id;
};

inline bool operator==(const timeout_msg& lhs, const timeout_msg& rhs) {
    return lhs.timeout_id == rhs.timeout_id;
}

inline bool operator!=(const timeout_msg& lhs, const timeout_msg& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Signalizes a newly accepted connection from a {@link broker}.
 */
struct new_connection_msg {
    /**
     * @brief The handle that accepted the new connection.
     */
    io::accept_handle source;
    /**
     * @brief The handle for the new connection.
     */
    io::connection_handle handle;
};

inline bool operator==(const new_connection_msg& lhs,
                       const new_connection_msg& rhs) {
    return lhs.source == rhs.source && lhs.handle == rhs.handle;
}

inline bool operator!=(const new_connection_msg& lhs,
                       const new_connection_msg& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Signalizes newly arrived data for a {@link broker}.
 */
struct new_data_msg {
    /**
     * @brief Handle to the related connection
     */
    io::connection_handle handle;
    /**
     * @brief Buffer containing the received data.
     */
    io::buffer buf;
};

inline bool operator==(const new_data_msg& lhs, const new_data_msg& rhs) {
    return lhs.handle == rhs.handle && lhs.buf == rhs.buf;
}

inline bool operator!=(const new_data_msg& lhs, const new_data_msg& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Signalizes that a {@link broker} connection has been closed.
 */
struct connection_closed_msg {
    /**
     * @brief Handle to the closed connection.
     */
    io::connection_handle handle;
};

inline bool operator==(const connection_closed_msg& lhs,
                       const connection_closed_msg& rhs) {
    return lhs.handle == rhs.handle;
}

inline bool operator!=(const connection_closed_msg& lhs,
                       const connection_closed_msg& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Signalizes that a {@link broker} acceptor has been closed.
 */
struct acceptor_closed_msg {
    /**
     * @brief Handle to the closed connection.
     */
    io::accept_handle handle;
};

inline bool operator==(const acceptor_closed_msg& lhs,
                       const acceptor_closed_msg& rhs) {
    return lhs.handle == rhs.handle;
}

inline bool operator!=(const acceptor_closed_msg& lhs,
                       const acceptor_closed_msg& rhs) {
    return !(lhs == rhs);
}

template<typename T>
typename std::enable_if<
    detail::tl_exists<
        detail::type_list<
            exit_msg,
            down_msg,
            sync_exited_msg
        >,
        detail::tbind<std::is_same, T>::template type
    >::value,
    bool
>::type
operator==(const T& lhs, const T& rhs) {
    return lhs.source == rhs.source && lhs.reason == rhs.reason;
}

template<typename T>
typename std::enable_if<
    detail::tl_exists<
        detail::type_list<
            exit_msg,
            down_msg,
            sync_exited_msg
        >,
        detail::tbind<std::is_same, T>::template type
    >::value,
    bool
>::type
operator!=(const T& lhs, const T& rhs) {
    return !(lhs == rhs);
}

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_SYSTEM_MESSAGES_HPP
