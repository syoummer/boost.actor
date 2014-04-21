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
