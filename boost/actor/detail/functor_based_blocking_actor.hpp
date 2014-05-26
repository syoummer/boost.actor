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


#ifndef FUNCTOR_BASED_BLOCKING_ACTOR_HPP
#define FUNCTOR_BASED_BLOCKING_ACTOR_HPP

#include "boost/actor/blocking_actor.hpp"

namespace boost {
namespace actor {
namespace detail {

class functor_based_blocking_actor : public blocking_actor {

 public:

    typedef std::function<void (blocking_actor*)> act_fun;

    template<typename F, typename... Ts>
    functor_based_blocking_actor(F f, Ts&&... vs) {
        blocking_actor* dummy = nullptr;
        create(dummy, f, std::forward<Ts>(vs)...);
    }

 protected:

    void act() override;

 private:

    void create(blocking_actor*, act_fun);

    template<class Actor, typename F, typename T0, typename... Ts>
    auto create(Actor* dummy, F f, T0&& v0, Ts&&... vs) ->
    typename std::enable_if<
        std::is_same<
            decltype(f(dummy, std::forward<T0>(v0), std::forward<Ts>(vs)...)),
            void
        >::value
    >::type {
        create(dummy, std::bind(f, std::placeholders::_1,
                                std::forward<T0>(v0), std::forward<Ts>(vs)...));
    }

    template<class Actor, typename F, typename T0, typename... Ts>
    auto create(Actor* dummy, F f, T0&& v0, Ts&&... vs) ->
    typename std::enable_if<
        std::is_same<
            decltype(f(std::forward<T0>(v0), std::forward<Ts>(vs)...)),
            void
        >::value
    >::type {
        std::function<void()> fun = std::bind(f, std::forward<T0>(v0),
                                              std::forward<Ts>(vs)...);
        create(dummy, [fun](Actor*) { fun(); });
    }

    act_fun m_act;

};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // FUNCTOR_BASED_BLOCKING_ACTOR_HPP
