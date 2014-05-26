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


#include <mutex>
#include <limits>
#include <stdexcept>

#include "boost/thread/locks.hpp"

#include "boost/actor/attachable.hpp"
#include "boost/actor/exit_reason.hpp"
#include "boost/actor/detail/actor_registry.hpp"

#include "boost/actor/detail/logging.hpp"
#include "boost/actor/detail/shared_spinlock.hpp"

namespace boost {
namespace actor {
namespace detail {

namespace {

typedef lock_guard<detail::shared_spinlock> exclusive_guard;
typedef shared_lock<detail::shared_spinlock> shared_guard;
typedef upgrade_lock<detail::shared_spinlock> upgrade_guard;
typedef upgrade_to_unique_lock<detail::shared_spinlock> upgrade_to_unique_guard;

} // namespace <anonymous>

actor_registry::~actor_registry() { }

actor_registry::actor_registry() : m_running(0), m_ids(1) { }

actor_registry::value_type actor_registry::get_entry(actor_id key) const {
    shared_guard guard(m_instances_mtx);
    auto i = m_entries.find(key);
    if (i != m_entries.end()) {
        return i->second;
    }
    BOOST_ACTOR_LOG_DEBUG("key not found: " << key);
    return {nullptr, exit_reason::not_exited};
}

void actor_registry::put(actor_id key, const abstract_actor_ptr& value) {
    bool add_attachable = false;
    if (value != nullptr) {
        upgrade_guard guard(m_instances_mtx);
        auto i = m_entries.find(key);
        if (i == m_entries.end()) {
            auto entry = std::make_pair(key,
                                        value_type(value,
                                                   exit_reason::not_exited));
            upgrade_to_unique_guard uguard(guard);
            add_attachable = m_entries.insert(entry).second;
        }
    }
    if (add_attachable) {
        BOOST_ACTOR_LOG_INFO("added actor with ID " << key);
        struct eraser : attachable {
            actor_id m_id;
            actor_registry* m_registry;
            eraser(actor_id id, actor_registry* s) : m_id(id), m_registry(s) { }
            void actor_exited(uint32_t reason) {
                m_registry->erase(m_id, reason);
            }
            bool matches(const token&) {
                return false;
            }
        };
        value->attach(attachable_ptr{new eraser(key, this)});
    }
}

void actor_registry::erase(actor_id key, uint32_t reason) {
    exclusive_guard guard(m_instances_mtx);
    auto i = m_entries.find(key);
    if (i != m_entries.end()) {
        auto& entry = i->second;
        BOOST_ACTOR_LOG_INFO("erased actor with ID " << key << ", reason " << reason);
        entry.first = nullptr;
        entry.second = reason;
    }
}

uint32_t actor_registry::next_id() {
    return m_ids.fetch_add(1);
}

void actor_registry::inc_running() {
#   if BOOST_ACTOR_LOG_LEVEL >= BOOST_ACTOR_DEBUG
    BOOST_ACTOR_LOG_DEBUG("new value = " << ++m_running);
#   else
    ++m_running;
#   endif
}

size_t actor_registry::running() const {
    return m_running.load();
}

void actor_registry::dec_running() {
    size_t new_val = --m_running;
    /*
    if (new_val == std::numeric_limits<size_t>::max()) {
        throw std::underflow_error("actor_count::dec()");
    }
    else*/ if (new_val <= 1) {
        std::unique_lock<std::mutex> guard(m_running_mtx);
        m_running_cv.notify_all();
    }
    BOOST_ACTOR_LOG_DEBUG(BOOST_ACTOR_ARG(new_val));
}

void actor_registry::await_running_count_equal(size_t expected) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(expected));
    std::unique_lock<std::mutex> guard{m_running_mtx};
    while (m_running != expected) {
        BOOST_ACTOR_LOG_DEBUG("count = " << m_running.load());
        m_running_cv.wait(guard);
    }
}

} } // namespace actor
} // namespace boost::detail
