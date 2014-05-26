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


#ifndef BOOST_ACTOR_ACCEPT_HANDLE_HPP
#define BOOST_ACTOR_ACCEPT_HANDLE_HPP

#include "boost/actor/io_handle.hpp"

namespace boost {
namespace actor {

/**
 * @brief Generic handle type for managing incoming connections.
 */
class accept_handle : public io_handle<accept_handle> {

    friend class io_handle<accept_handle>;

    typedef io_handle<accept_handle> super;

 public:

    accept_handle() = default;

 private:

    inline accept_handle(int64_t handle_id) : super{handle_id} { }

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_ACCEPT_HANDLE_HPP
