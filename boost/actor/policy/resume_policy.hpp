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


#ifndef BOOST_ACTOR_RESUME_POLICY_HPP
#define BOOST_ACTOR_RESUME_POLICY_HPP

#include "boost/actor/resumable.hpp"

// this header consists all type definitions needed to
// implement the resume_policy trait

namespace boost {
namespace actor {

class execution_unit;
class duration;

namespace policy {

/**
 * @brief The resume_policy <b>concept</b> class. Please note that this
 *        class is <b>not</b> implemented. It only explains the all
 *        required member function and their behavior for any resume policy.
 */
class resume_policy {

 public:

    /**
     * @brief Resumes the actor by reading a new message <tt>msg</tt> and then
     *        calling <tt>self->invoke(msg)</tt>. This process is repeated
     *        until either no message is left in the actor's mailbox or the
     *        actor finishes execution.
     */
    template<class Actor>
    resumable::resume_result resume(Actor* self, execution_unit*);

    /**
     * @brief Waits unconditionally until the actor is ready to resume.
     * @note This member function must raise a compiler error if the resume
     *       strategy cannot be used to implement blocking actors.
     *
     * This member function calls {@link scheduling_policy::await_data}
     */
    template<class Actor>
    bool await_ready(Actor* self);

};

} // namespace policy
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_RESUME_POLICY_HPP
