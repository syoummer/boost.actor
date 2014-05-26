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


#ifndef BOOST_ACTOR_TYPED_EVENT_BASED_ACTOR_HPP
#define BOOST_ACTOR_TYPED_EVENT_BASED_ACTOR_HPP

#include "boost/actor/replies_to.hpp"
#include "boost/actor/local_actor.hpp"
#include "boost/actor/typed_behavior.hpp"

#include "boost/actor/mixin/sync_sender.hpp"
#include "boost/actor/mixin/mailbox_based.hpp"
#include "boost/actor/mixin/behavior_stack_based.hpp"

namespace boost {
namespace actor {

/**
 * @brief A cooperatively scheduled, event-based actor implementation
 *        with strong type checking.
 *
 * This is the recommended base class for user-defined actors and is used
 * implicitly when spawning typed, functor-based actors without the
 * {@link blocking_api} flag.
 *
 * @extends local_actor
 */
template<typename... Rs>
class typed_event_based_actor
        : public extend<local_actor, typed_event_based_actor<Rs...>>::template
                 with<mixin::mailbox_based,
                      mixin::behavior_stack_based<
                          typed_behavior<Rs...>
                      >::template impl,
                      mixin::sync_sender<
                          nonblocking_response_handle_tag
                      >::template impl> {

 public:

    typed_event_based_actor() : m_initialized(false) { }

    typedef detail::type_list<Rs...> signatures;

    typedef typed_behavior<Rs...> behavior_type;

    std::set<std::string> interface() const override {
        return {detail::to_uniform_name<Rs>()...};
    }

 protected:

    virtual behavior_type make_behavior() = 0;

    bool m_initialized;

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_TYPED_EVENT_BASED_ACTOR_HPP
