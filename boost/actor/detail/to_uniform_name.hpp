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


#ifndef BOOST_ACTOR_TO_UNIFORM_NAME_HPP
#define BOOST_ACTOR_TO_UNIFORM_NAME_HPP

#include <string>
#include <typeinfo>

namespace boost {
namespace actor {
namespace detail {

std::string to_uniform_name(const std::string& demangled_name);
std::string to_uniform_name(const std::type_info& tinfo);

template<class T>
inline std::string to_uniform_name() { return to_uniform_name(typeid(T)); }

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_TO_UNIFORM_NAME_HPP
