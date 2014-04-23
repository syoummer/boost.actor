/******************************************************************************\
 *           ___        __                                                    *
 *          /\_ \    __/\ \                                                   *
 *          \//\ \  /\_\ \ \____    ___   _____   _____      __               *
 *            \ \ \ \/\ \ \ '__`\  /'___\/\ '__`\/\ '__`\  /'__`\             *
 *             \_\ \_\ \ \ \ \L\ \/\ \__/\ \ \L\ \ \ \L\ \/\ \L\.\_           *
 *             /\____\\ \_\ \_,__/\ \____\\ \ ,__/\ \ ,__/\ \__/.\_\          *
 *             \/____/ \/_/\/___/  \/____/ \ \ \/  \ \ \/  \/__/\/_/          *
 *                                          \ \_\   \ \_\                     *
 *                                           \/_/    \/_/                     *
 *                                                                            *
 * Copyright (C) 2011-2013                                                    *
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * This file is part of libcppa.                                              *
 * libcppa is free software: you can redistribute it and/or modify it under   *
 * the terms of the GNU Lesser General Public License as published by the     *
 * Free Software Foundation; either version 2.1 of the License,               *
 * or (at your option) any later version.                                     *
 *                                                                            *
 * libcppa is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with libcppa. If not, see <http://www.gnu.org/licenses/>.            *
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

#include "boost/actor/atom.hpp"
#include "boost/actor/actor.hpp"
#include "boost/actor/logging.hpp"
#include "boost/actor/any_tuple.hpp"
#include "boost/actor/announce.hpp"
#include "boost/actor/any_tuple.hpp"
#include "boost/intrusive_ptr.hpp"
#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/util/duration.hpp"

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
                                  std::unique_ptr<uniform_type_info> utype) {
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
