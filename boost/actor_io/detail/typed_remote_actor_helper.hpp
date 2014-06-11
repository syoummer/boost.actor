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


#ifndef BOOST_ACTOR_DETAIL_TYPED_REMOTE_ACTOR_HELPER_HPP
#define BOOST_ACTOR_DETAIL_TYPED_REMOTE_ACTOR_HELPER_HPP

#include "boost/actor/actor_cast.hpp"
#include "boost/actor/typed_actor.hpp"

#include "boost/actor/detail/type_list.hpp"

#include "boost/actor_io/detail/remote_actor_impl.hpp"

namespace boost {
namespace actor_io {
namespace detail {

template<class List>
struct typed_remote_actor_helper;

template<typename... Ts>
struct typed_remote_actor_helper<actor::detail::type_list<Ts...>> {
    typedef actor::typed_actor<Ts...> return_type;
    template<typename... Vs>
    return_type operator()(Vs&&... vs) {
        auto iface = return_type::get_interface();
        auto tmp = remote_actor_impl(std::forward<Vs>(vs)..., std::move(iface));
        return actor::actor_cast<return_type>(tmp);
    }
};

} // namespace detail
} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_TYPED_REMOTE_ACTOR_HELPER_HPP
