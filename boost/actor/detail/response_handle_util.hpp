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


#ifndef BOOST_ACTOR_DETAIL_RESPONSE_FUTURE_UTIL_HPP
#define BOOST_ACTOR_DETAIL_RESPONSE_FUTURE_UTIL_HPP

#include "boost/actor/on.hpp"
#include "boost/actor/skip_message.hpp"
#include "boost/actor/system_messages.hpp"

#include "boost/actor/detail/type_traits.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename Actor, typename... Fs>
behavior fs2bhvr(Actor* self, Fs... fs) {
    auto handle_sync_timeout = [self]() -> skip_message_t {
        self->handle_sync_timeout();
        return {};
    };
    return behavior{
        on<sync_timeout_msg>() >> handle_sync_timeout,
        on<unit_t>() >> skip_message,
        on<sync_exited_msg>() >> skip_message,
        (on(any_vals, arg_match) >> std::move(fs))...
    };
}

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_RESPONSE_FUTURE_UTIL_HPP
