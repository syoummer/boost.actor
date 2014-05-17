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


#ifndef BOOST_ACTOR_GROUP_MANAGER_HPP
#define BOOST_ACTOR_GROUP_MANAGER_HPP

#include <map>
#include <mutex>
#include <thread>

#include "boost/actor/abstract_group.hpp"
#include "boost/actor/detail/shared_spinlock.hpp"

#include "boost/actor/detail/singleton_mixin.hpp"

namespace boost {
namespace actor {
namespace detail {

class group_manager : public singleton_mixin<group_manager> {

    friend class singleton_mixin<group_manager>;

 public:

    ~group_manager();

    group get(const std::string& module_name,
              const std::string& group_identifier);

    group anonymous();

    void add_module(abstract_group::unique_module_ptr);

    abstract_group::module_ptr get_module(const std::string& module_name);

 private:

    typedef std::map<std::string, abstract_group::unique_module_ptr> modules_map;

    modules_map m_mmap;
    std::mutex m_mmap_mtx;

    group_manager();

};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_GROUP_MANAGER_HPP
