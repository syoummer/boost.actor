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


#ifndef TIMEOUT_DEFINITION_HPP
#define TIMEOUT_DEFINITION_HPP

#include <functional>

#include "boost/actor/duration.hpp"

namespace boost {
namespace actor {

namespace detail { class behavior_impl; }

template<typename F>
struct timeout_definition {
    static constexpr bool may_have_timeout = true;
    duration timeout;
    F handler;
    detail::behavior_impl* as_behavior_impl() const;
};

typedef timeout_definition<std::function<void()> > generic_timeout_definition;

} // namespace actor
} // namespace boost

#endif // TIMEOUT_DEFINITION_HPP
