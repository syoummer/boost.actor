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


#ifndef BOOST_ACTOR_DETAIL_EXECUTION_UNIT_HPP
#define BOOST_ACTOR_DETAIL_EXECUTION_UNIT_HPP

namespace boost {
namespace actor {

class resumable;

/*
 * @brief Identifies an execution unit, e.g., a worker thread of the scheduler.
 */
class execution_unit {

 public:

    virtual ~execution_unit();

    /*
     * @brief Enqueues @p ptr to the job list of the execution unit.
     * @warning Must only be called from a {@link resumable} currently
     *          executed by this execution unit.
     */
    virtual void exec_later(resumable* ptr) = 0;

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_EXECUTION_UNIT_HPP
