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


#ifndef BOOST_ACTOR_SHARED_SPINLOCK_HPP
#define BOOST_ACTOR_SHARED_SPINLOCK_HPP

#include <atomic>
#include <cstddef>

namespace boost {
namespace actor {
namespace detail {

/**
 * @brief A spinlock implementation providing shared and exclusive locking.
 */
class shared_spinlock {

    std::atomic<long> m_flag;

 public:

    shared_spinlock();

    void lock();
    void unlock();
    bool try_lock();

    void lock_shared();
    void unlock_shared();
    bool try_lock_shared();

    void lock_upgrade();
    void unlock_upgrade();
    void unlock_upgrade_and_lock();
    void unlock_and_lock_upgrade();

};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_SHARED_SPINLOCK_HPP
