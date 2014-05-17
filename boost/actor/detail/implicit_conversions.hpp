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


#ifndef BOOST_ACTOR_IMPLICIT_CONVERSIONS_HPP
#define BOOST_ACTOR_IMPLICIT_CONVERSIONS_HPP

#include <string>
#include <type_traits>

#include "boost/actor/detail/type_traits.hpp"

namespace boost { namespace actor { class local_actor; } }

namespace boost {
namespace actor {
namespace detail {

template<typename T>
struct implicit_conversions {

    typedef typename detail::replace_type<
                T,
                std::string,
                std::is_same<T, const char*>,
                std::is_same<T, char*>,
                std::is_same<T, char[]>,
                detail::is_array_of<T, char>,
                detail::is_array_of<T, const char>
            >::type
            subtype1;

    typedef typename detail::replace_type<
                subtype1,
                std::u16string,
                std::is_same<subtype1, const char16_t*>,
                std::is_same<subtype1, char16_t*>,
                detail::is_array_of<subtype1, char16_t>
            >::type
            subtype2;

    typedef typename detail::replace_type<
                subtype2,
                std::u32string,
                std::is_same<subtype2, const char32_t*>,
                std::is_same<subtype2, char32_t*>,
                detail::is_array_of<subtype2, char32_t>
            >::type
            subtype3;

    typedef typename detail::replace_type<
                subtype3,
                actor,
                std::is_convertible<T, abstract_actor*>,
                std::is_same<scoped_actor, T>
            >::type
            type;

};

template<typename T>
struct strip_and_convert {
    typedef typename implicit_conversions<typename detail::rm_const_and_ref<T>::type>::type
            type;
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_IMPLICIT_CONVERSIONS_HPP
