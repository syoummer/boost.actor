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


#ifndef BOOST_ACTOR_FROM_STRING_HPP
#define BOOST_ACTOR_FROM_STRING_HPP

#include <string>
#include <typeinfo>
#include <exception>

#include "boost/actor/uniform_type_info.hpp"

namespace boost {
namespace actor {

/**
 * @brief Converts a string created by {@link cppa::to_string to_string}
 *        to its original value.
 * @param what String representation of a serialized value.
 * @returns An {@link cppa::object object} instance that contains
 *          the deserialized value.
 */
uniform_value from_string(const std::string& what);

/**
 * @brief Convenience function that deserializes a value from @p what and
 *        converts the result to @p T.
 * @throws std::logic_error if the result is not of type @p T.
 * @returns The deserialized value as instance of @p T.
 */
/*
template<typename T>
T from_string(const std::string& what) {
    any o = from_string(what);
    const std::type_info& tinfo = typeid(T);
    if (tinfo == *(o.type())) {
        return std::move(get_ref<T>(o));
    }
    else {
        std::string error_msg = "expected type name ";
        error_msg += uniform_typeid(tinfo)->name();
        error_msg += " found ";
        error_msg += o.type()->name();
        throw std::logic_error(error_msg);
    }
}
*/

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_FROM_STRING_HPP
