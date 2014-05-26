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


#ifndef BOOST_ACTOR_IO_STREAM_HPP
#define BOOST_ACTOR_IO_STREAM_HPP

#include "boost/actor_io/input_stream.hpp"
#include "boost/actor_io/output_stream.hpp"

namespace boost {
namespace actor_io {

/**
 * @brief A stream capable of both reading and writing.
 */
class stream : public input_stream, public output_stream {

 public:

    ~stream();

};

/**
 * @brief An IO stream pointer.
 */
typedef intrusive_ptr<stream> stream_ptr;

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_IO_STREAM_HPP
