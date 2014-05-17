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


#ifndef BOOST_ACTOR_THREADLESS_HPP
#define BOOST_ACTOR_THREADLESS_HPP

#include "boost/actor/atom.hpp"
#include "boost/actor/behavior.hpp"
#include "boost/actor/duration.hpp"

#include "boost/actor/policy/invoke_policy.hpp"

namespace boost {
namespace actor { namespace policy {

/**
 * @brief An actor that is scheduled or otherwise managed.
 */
class sequential_invoke : public invoke_policy<sequential_invoke> {

 public:

    inline bool hm_should_skip(mailbox_element*) {
        return false;
    }

    template<class Actor>
    inline mailbox_element* hm_begin(Actor* self, mailbox_element* node) {
        auto previous = self->current_node();
        self->current_node(node);
        return previous;
    }

    template<class Actor>
    inline void hm_cleanup(Actor* self, mailbox_element*) {
        self->current_node(self->dummy_node());
    }

    template<class Actor>
    inline void hm_revert(Actor* self, mailbox_element* previous) {
        self->current_node(previous);
    }

};

} } // namespace actor
} // namespace boost::policy

#endif // BOOST_ACTOR_THREADLESS_HPP
