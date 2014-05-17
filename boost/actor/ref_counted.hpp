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


#ifndef BOOST_ACTOR_REF_COUNTED_HPP
#define BOOST_ACTOR_REF_COUNTED_HPP

#include <atomic>
#include <cstddef>

#include "boost/actor/memory_managed.hpp"

namespace boost {
namespace actor {

/**
 * @brief A (thread safe) base class for reference counted objects
 *        with an atomic reference count.
 *
 * Serves the requirements of {@link intrusive_ptr}.
 * @relates intrusive_ptr
 */
class ref_counted : public memory_managed {

 public:

    ref_counted();

    ~ref_counted();

    /**
     * @brief Increases reference count by one.
     */
    inline void ref() { ++m_rc; }

    /**
     * @brief Decreases reference count by one and calls
     *        @p request_deletion when it drops to zero.
     */
    inline void deref() { if (--m_rc == 0) request_deletion(); }

    /**
     * @brief Queries whether there is exactly one reference.
     */
    inline bool unique() const { return m_rc == 1; }

    inline size_t get_reference_count() const { return m_rc; }

 private:

    std::atomic<size_t> m_rc;

};

inline void intrusive_ptr_add_ref(ref_counted* p) {
    p->ref();
}

inline void intrusive_ptr_release(ref_counted* p) {
    p->deref();
}

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_REF_COUNTED_HPP
