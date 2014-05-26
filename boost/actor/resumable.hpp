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


#ifndef BOOST_ACTOR_RESUMABLE_HPP
#define BOOST_ACTOR_RESUMABLE_HPP

namespace boost {
namespace actor {

class execution_unit;

/**
 * @brief A cooperatively executed task managed by one or more instances of
 *        {@link execution_unit}.
 */
class resumable {

 public:

    enum resume_result {
        resume_later,
        done,
        shutdown_execution_unit
    };

    resumable();

    virtual ~resumable();

    /**
     * @brief Initializes this object, e.g., by increasing the
     *        the reference count.
     */
    virtual void attach_to_scheduler() = 0;

    /**
     * @brief Uninitializes this object, e.g., by decrementing the
     *        the reference count.
     */
    virtual void detach_from_scheduler() = 0;

    /**
     * @brief Resume any pending computation until it is either finished
     *        or needs to be re-scheduled later.
     */
    virtual resume_result resume(execution_unit*) = 0;

 protected:

    bool m_hidden;

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_RESUMABLE_HPP
