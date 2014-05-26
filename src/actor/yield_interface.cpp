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


#include <memory>

#include "boost/actor/detail/cs_thread.hpp"
#include "boost/actor/detail/yield_interface.hpp"

namespace boost {
namespace actor {
namespace detail {

namespace {

__thread detail::yield_state* t_ystate = nullptr;
__thread detail::cs_thread* t_caller = nullptr;
__thread detail::cs_thread* t_callee = nullptr;

constexpr const char* names_table[] = {
    "yield_state::invalid",
    "yield_state::ready",
    "yield_state::blocked",
    "yield_state::done"
};

} // namespace <anonymous>

void yield(yield_state ystate) {
    *t_ystate = ystate;
    detail::cs_thread::swap(*t_callee, *t_caller);
}

yield_state call(detail::cs_thread* what, detail::cs_thread* from) {
    yield_state result;
    t_ystate = &result;
    t_caller = from;
    t_callee = what;
    detail::cs_thread::swap(*from, *what);
    return result;
}

} } // namespace actor
} // namespace boost::detail

namespace boost {
namespace actor {

std::string to_string(detail::yield_state ys) {
    auto i = static_cast<size_t>(ys);
    return (i < sizeof(detail::names_table)) ? detail::names_table[i]
                                             : "{illegal yield_state}";
}

} // namespace actor
} // namespace boost
