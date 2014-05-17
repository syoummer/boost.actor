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


#ifndef BOOST_ACTOR_INPUT_STREAM_HPP
#define BOOST_ACTOR_INPUT_STREAM_HPP

#include "boost/intrusive_ptr.hpp"

#include "boost/actor/ref_counted.hpp"

#include "boost/actor/io/platform.hpp"

namespace boost {
namespace actor {
namespace io {

/**
 * @brief An abstract input stream interface.
 */
class input_stream : public virtual ref_counted {

 public:

    ~input_stream();

    /**
     * @brief Returns the internal file descriptor. This descriptor is needed
     *        for socket multiplexing using select().
     */
    virtual native_socket_type read_handle() const = 0;

    /**
     * @brief Reads exactly @p num_bytes from the data source and blocks the
     *        caller if needed.
     * @throws std::ios_base::failure
     */
    virtual void read(void* buf, size_t num_bytes) = 0;

    /**
     * @brief Tries to read up to @p num_bytes from the data source.
     * @returns The number of read bytes.
     * @throws std::ios_base::failure
     */
    virtual size_t read_some(void* buf, size_t num_bytes) = 0;

};

/**
 * @brief An input stream pointer.
 */
typedef intrusive_ptr<input_stream> input_stream_ptr;

} // namespace io
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_INPUT_STREAM_HPP
