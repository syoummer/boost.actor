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


#ifndef BOOST_ACTOR_TYPE_PAIR_HPP
#define BOOST_ACTOR_TYPE_PAIR_HPP

namespace boost {
namespace actor {
namespace detail {

/**
 * @ingroup MetaProgramming
 * @brief A pair of two types.
 */
template<typename First, typename Second>
struct type_pair {
    typedef First first;
    typedef Second second;
};

template<typename First, typename Second>
struct to_type_pair {
    typedef type_pair<First, Second> type;
};

template<class What>
struct is_type_pair {
    static constexpr bool value = false;
};

template<typename First, typename Second>
struct is_type_pair<type_pair<First, Second> > {
    static constexpr bool value = true;
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_TYPE_PAIR_HPP
