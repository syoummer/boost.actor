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


#ifndef BOOST_ACTOR_ACCEPTOR_HPP
#define BOOST_ACTOR_ACCEPTOR_HPP

#include <memory>

#include "boost/optional.hpp"

#include "boost/actor/io/platform.hpp"
#include "boost/actor/io/input_stream.hpp"
#include "boost/actor/io/output_stream.hpp"

namespace boost {
namespace actor {
namespace io {

/**
 * @brief A pair of input and output stream pointers.
 */
typedef std::pair<input_stream_ptr, output_stream_ptr> stream_ptr_pair;

/**
 * @brief Accepts connections from client processes.
 */
class acceptor {

 public:

    virtual ~acceptor();

    /**
     * @brief Returns the internal file descriptor. This descriptor is needed
     *        for socket multiplexing using select().
     */
    virtual native_socket_type file_handle() const = 0;

    /**
     * @brief Accepts a new connection and returns an input/output stream pair.
     * @note This member function blocks until a new connection was established.
     */
    virtual stream_ptr_pair accept_connection() = 0;

    /**
     * @brief Tries to accept a new connection but immediately returns if
     *        there is no pending connection.
     */
    virtual optional<stream_ptr_pair> try_accept_connection() = 0;

};

typedef std::unique_ptr<acceptor> acceptor_uptr;


} // namespace io
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_ACCEPTOR_HPP
