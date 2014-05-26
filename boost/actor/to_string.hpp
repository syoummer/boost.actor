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


#ifndef BOOST_ACTOR_TO_STRING_HPP
#define BOOST_ACTOR_TO_STRING_HPP

#include <type_traits>

#include "boost/actor/atom.hpp" // included for to_string(atom_value)
#include "boost/actor/actor.hpp"
#include "boost/actor/group.hpp"
#include "boost/actor/channel.hpp"
#include "boost/actor/node_id.hpp"
#include "boost/actor/anything.hpp"
#include "boost/actor/message.hpp"
#include "boost/intrusive_ptr.hpp"
#include "boost/actor/abstract_group.hpp"
#include "boost/actor/message_header.hpp"
#include "boost/actor/uniform_type_info.hpp"

namespace std { class exception; }

namespace boost {
namespace actor {

namespace detail {

std::string to_string_impl(const void* what, const uniform_type_info* utype);

template<typename T>
inline std::string to_string_impl(const T& what) {
    return to_string_impl(&what, uniform_typeid<T>());
}

} // namespace detail

inline std::string to_string(const message& what) {
    return detail::to_string_impl(what);
}

inline std::string to_string(msg_hdr_cref what) {
    return detail::to_string_impl(what);
}

inline std::string to_string(const actor& what) {
    return detail::to_string_impl(what);
}

inline std::string to_string(const actor_addr& what) {
    return detail::to_string_impl(what);
}

inline std::string to_string(const group& what) {
    return detail::to_string_impl(what);
}

inline std::string to_string(const channel& what) {
    return detail::to_string_impl(what);
}

// implemented in node_id.cpp
std::string to_string(const node_id& what);

// implemented in node_id.cpp
std::string to_string(const node_id_ptr& what);

/*
inline std::string to_string(const any& what) {
    return detail::to_string_impl(what.value(), what.type());
}
*/

/**
 * @brief Converts @p e to a string including the demangled type of e
 *        and @p e.what().
 */
std::string to_verbose_string(const std::exception& e);

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_TO_STRING_HPP
