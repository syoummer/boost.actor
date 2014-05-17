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


#include "boost/actor/to_string.hpp"

#include "boost/actor/config.hpp"
#include "boost/actor/message_handler.hpp"

namespace boost {
namespace actor {

message_handler::message_handler(impl_ptr ptr) : m_impl(std::move(ptr)) { }

void detail::behavior_impl::handle_timeout() { }

} // namespace actor
} // namespace boost

namespace boost {
namespace actor {
namespace detail {

message_handler combine(behavior_impl_ptr lhs, behavior_impl_ptr rhs) {
    return {lhs->or_else(rhs)};
}

behavior_impl_ptr extract(const message_handler& arg) {
    return arg.as_behavior_impl();
}

} } // namespace actor
} // namespace boost::detail
