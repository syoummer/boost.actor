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


#ifndef BOOST_ACTOR_SINGLETON_MANAGER_HPP
#define BOOST_ACTOR_SINGLETON_MANAGER_HPP

#include <atomic>

namespace boost { namespace actor { class logging; } }

namespace boost { namespace actor { namespace scheduler { class coordinator; } } }

namespace boost { namespace actor { namespace io { class middleman; } } }

namespace boost { namespace actor { namespace opencl { class opencl_metainfo; } } }

namespace boost {
namespace actor {
namespace detail {

class group_manager;
class message_data;
class actor_registry;
class uniform_type_info_map;

class singleton_manager {

    singleton_manager() = delete;

 public:

    static void shutdown();

    static logging* get_logger();

    static scheduler::coordinator* get_scheduling_coordinator();

    static group_manager* get_group_manager();

    static actor_registry* get_actor_registry();

    static io::middleman* get_middleman();

    static uniform_type_info_map* get_uniform_type_info_map();

    static message_data* get_tuple_dummy();

    static opencl::opencl_metainfo* get_opencl_metainfo();

 private:

    /*
     * @brief Type @p T has to provide: <tt>static T* create_singleton()</tt>,
     *        <tt>void initialize()</tt>, <tt>void destroy()</tt>,
     *        and <tt>dispose()</tt>.
     * The constructor of T shall be lightweigt, since more than one object
     * might get constructed initially.
     * <tt>dispose()</tt> is called on objects with failed CAS operation.
     * <tt>initialize()</tt> is called on objects with succeeded CAS operation.
     * <tt>destroy()</tt> is called during shutdown on initialized objects.
     *
     * Both <tt>dispose</tt> and <tt>destroy</tt> must delete the object
     * eventually.
     */
    template<typename T>
    static T* lazy_get(std::atomic<T*>& ptr) {
        T* result = ptr.load();
        while (result == nullptr) {
            auto tmp = T::create_singleton();
            // double check if singleton is still undefined
            if (ptr.load() == nullptr) {
                tmp->initialize();
                if (ptr.compare_exchange_weak(result, tmp)) {
                    result = tmp;
                }
                else tmp->destroy();
            }
            else tmp->dispose();
        }
        return result;
    }

    template<typename T>
    static void destroy(std::atomic<T*>& ptr) {
        for (;;) {
            auto p = ptr.load();
            if (p == nullptr) {
                return;
            }
            else if (ptr.compare_exchange_weak(p, nullptr)) {
                p->destroy();
                ptr = nullptr;
                return;
            }
        }
    }

};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_SINGLETON_MANAGER_HPP
