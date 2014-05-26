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


#ifndef BOOST_ACTOR_FSM_ACTOR_HPP
#define BOOST_ACTOR_FSM_ACTOR_HPP

#include <utility>
#include <type_traits>

#include "boost/actor/event_based_actor.hpp"

namespace boost {
namespace actor {

/**
 * @brief A base class for state-based actors using the
 *        Curiously Recurring Template Pattern
 *        to initialize the derived actor with its @p init_state member.
 * @tparam Derived Direct subclass of @p sb_actor.
 */
template<class Derived, class Base = event_based_actor>
class sb_actor : public Base {

    static_assert(std::is_base_of<event_based_actor, Base>::value,
                  "Base must be event_based_actor or a derived type");

 protected:

    typedef sb_actor combined_type;

 public:

    /**
     * @brief Overrides {@link event_based_actor::make_behavior()} and sets
     *        the initial actor behavior to <tt>Derived::init_state</tt>.
     */
    behavior make_behavior() override {
        return static_cast<Derived*>(this)->init_state;
    }

 protected:

    template<typename... Ts>
    sb_actor(Ts&&... args) : Base(std::forward<Ts>(args)...) { }

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_FSM_ACTOR_HPP
