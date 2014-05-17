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


#include "boost/actor/config.hpp"

#include <map>
#include <set>
#include <locale>
#include <string>
#include <atomic>
#include <limits>
#include <cstring>
#include <cstdint>
#include <type_traits>

#include "boost/intrusive_ptr.hpp"

#include "boost/actor/atom.hpp"
#include "boost/actor/actor.hpp"
#include "boost/actor/logging.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/announce.hpp"
#include "boost/actor/duration.hpp"
#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/detail/demangle.hpp"
#include "boost/actor/detail/actor_registry.hpp"
#include "boost/actor/detail/to_uniform_name.hpp"
#include "boost/actor/detail/singleton_manager.hpp"
#include "boost/actor/detail/uniform_type_info_map.hpp"

namespace boost {
namespace actor {

namespace { inline detail::uniform_type_info_map& uti_map() {
    return *detail::singleton_manager::get_uniform_type_info_map();
} } // namespace <anonymous>

uniform_value_t::~uniform_value_t() { }

const uniform_type_info* announce(const std::type_info&,
                                  uniform_type_info_ptr utype) {
    return uti_map().insert(std::move(utype));
}

uniform_type_info::~uniform_type_info() { }

const uniform_type_info* uniform_type_info::from(const std::type_info& tinf) {
    auto result = uti_map().by_rtti(tinf);
    if (result == nullptr) {
        std::string error = "uniform_type_info::by_type_info(): ";
        error += detail::to_uniform_name(tinf);
        error += " is an unknown typeid name";
        BOOST_ACTOR_LOGM_ERROR("cppa::uniform_type_info", error);
        throw std::runtime_error(error);
    }
    return result;
}

const uniform_type_info* uniform_type_info::from(const std::string& name) {
    auto result = uti_map().by_uniform_name(name);
    if (result == nullptr) {
        throw std::runtime_error(name + " is an unknown typeid name");
    }
    return result;
}

uniform_value uniform_type_info::deserialize(deserializer* from) const {
    auto uval = create();
    deserialize(uval->val, from);
    return std::move(uval);
}

std::vector<const uniform_type_info*> uniform_type_info::instances() {
    return uti_map().get_all();
}

const uniform_type_info* uniform_typeid(const std::type_info& tinfo) {
    return uniform_type_info::from(tinfo);
}

} // namespace actor
} // namespace boost
