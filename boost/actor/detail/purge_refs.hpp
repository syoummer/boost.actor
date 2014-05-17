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


#ifndef BOOST_ACTOR_PURGE_REFS_HPP
#define BOOST_ACTOR_PURGE_REFS_HPP

#include <functional>

#include "boost/actor/detail/type_traits.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename T>
struct purge_refs_impl {
    typedef T type;
};

template<typename T>
struct purge_refs_impl<std::reference_wrapper<T> > {
    typedef T type;
};

template<typename T>
struct purge_refs_impl<std::reference_wrapper<const T> > {
    typedef T type;
};

/**
 * @brief Removes references and reference wrappers.
 */
template<typename T>
struct purge_refs {
    typedef typename purge_refs_impl<typename detail::rm_const_and_ref<T>::type>::type type;
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_PURGE_REFS_HPP
