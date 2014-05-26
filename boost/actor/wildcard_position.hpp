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


#ifndef BOOST_ACTOR_WILDCARD_POSITION_HPP
#define BOOST_ACTOR_WILDCARD_POSITION_HPP

#include <type_traits>

#include "boost/actor/anything.hpp"
#include "boost/actor/detail/type_list.hpp"

namespace boost {
namespace actor {

/**
 * @brief Denotes the position of {@link cppa::anything anything} in a
 *        template parameter pack.
 */
enum class wildcard_position {
    nil,
    trailing,
    leading,
    in_between,
    multiple
};

/**
 * @brief Gets the position of {@link cppa::anything anything} from the
 *        type list @p Types.
 * @tparam A template parameter pack as {@link cppa::detail::type_list type_list}.
 */
template<typename Types>
constexpr wildcard_position get_wildcard_position() {
    return detail::tl_count<Types, is_anything>::value > 1
           ? wildcard_position::multiple
           : (detail::tl_count<Types, is_anything>::value == 1
              ? (std::is_same<typename detail::tl_head<Types>::type, anything>::value
                 ? wildcard_position::leading
                 : (std::is_same<typename detail::tl_back<Types>::type, anything>::value
                    ? wildcard_position::trailing
                    : wildcard_position::in_between))
              : wildcard_position::nil);
}

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_WILDCARD_POSITION_HPP
