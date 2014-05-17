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


#ifndef BOOST_ACTOR_OUTPUT_STREAM_HPP
#define BOOST_ACTOR_OUTPUT_STREAM_HPP

#include "boost/intrusive_ptr.hpp"

#include "boost/actor/ref_counted.hpp"

#include "boost/actor/io/platform.hpp"

namespace boost {
namespace actor {
namespace io {

/**
 * @brief An abstract output stream interface.
 */
class output_stream : public virtual ref_counted {

 public:

    ~output_stream();

    /**
     * @brief Returns the internal file descriptor. This descriptor is needed
     *        for socket multiplexing using select().
     */
    virtual native_socket_type write_handle() const = 0;

    /**
     * @brief Writes @p num_bytes bytes from @p buf to the data sink.
     * @note This member function blocks until @p num_bytes were successfully
     *       written.
     * @throws std::ios_base::failure
     */
    virtual void write(const void* buf, size_t num_bytes) = 0;

    /**
     * @brief Tries to write up to @p num_bytes bytes from @p buf.
     * @returns The number of written bytes.
     * @throws std::ios_base::failure
     */
    virtual size_t write_some(const void* buf, size_t num_bytes) = 0;

};

/**
 * @brief An output stream pointer.
 */
typedef intrusive_ptr<output_stream> output_stream_ptr;

} // namespace io
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_OUTPUT_STREAM_HPP
