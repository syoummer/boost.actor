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


#include <utility>
#include <iostream>

#include "boost/thread/locks.hpp"

#include "boost/actor/atom.hpp"
#include "boost/actor/to_string.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/scheduler.hpp"
#include "boost/actor/actor_proxy.hpp"
#include "boost/actor/exit_reason.hpp"

#include "boost/actor_io/middleman.hpp"

#include "boost/actor/detail/singletons.hpp"

using namespace std;

namespace boost {
namespace actor {

actor_proxy::anchor::anchor(actor_proxy* instance) : m_ptr(instance) { }

actor_proxy::anchor::~anchor() { }

bool actor_proxy::anchor::expired() const {
    return !m_ptr;
}

actor_proxy_ptr actor_proxy::anchor::get() {
    actor_proxy_ptr result;
    { // lifetime scope of guard
        shared_lock<detail::shared_spinlock> guard{m_lock};
        auto ptr = m_ptr.load();
        if (ptr) result.reset(ptr);
    }
    return result;
}

bool actor_proxy::anchor::try_expire() {
    std::lock_guard<detail::shared_spinlock> guard{m_lock};
    // double-check reference count
    if (m_ptr.load()->get_reference_count() == 0) {
        m_ptr = nullptr;
        return true;
    }
    return false;
}

actor_proxy::~actor_proxy() { }

actor_proxy::actor_proxy(actor_id aid, node_id nid)
: super(aid, nid), m_anchor(new anchor{this}) { }

void actor_proxy::request_deletion() {
    if (m_anchor->try_expire()) delete this;
}

} // namespace actor
} // namespace boost
