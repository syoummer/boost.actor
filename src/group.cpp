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


#include "boost/actor/group.hpp"
#include "boost/actor/channel.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/singletons.hpp"

#include "boost/actor/detail/group_manager.hpp"

namespace boost {
namespace actor {

group::group(const invalid_group_t&) : m_ptr(nullptr) { }

group::group(abstract_group_ptr ptr) : m_ptr(std::move(ptr)) { }

group& group::operator=(const invalid_group_t&) {
    m_ptr.reset();
    return *this;
}

intptr_t group::compare(const group& other) const {
    return channel::compare(m_ptr.get(), other.m_ptr.get());
}

group group::get(const std::string& arg0, const std::string& arg1) {
    return get_group_manager()->get(arg0, arg1);
}

group group::anonymous() {
    return get_group_manager()->anonymous();
}

void group::add_module(abstract_group::unique_module_ptr ptr) {
    get_group_manager()->add_module(std::move(ptr));
}

abstract_group::module_ptr group::get_module(const std::string& module_name) {
    return get_group_manager()->get_module(module_name);
}

} // namespace actor
} // namespace boost
