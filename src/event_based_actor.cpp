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
 * Copyright (C) 2011, 2012                                                   *
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * This file is part of libcppa.                                              *
 * libcppa is free software: you can redistribute it and/or modify it under   *
 * the terms of the GNU Lesser General Public License as published by the     *
 * Free Software Foundation, either version 3 of the License                  *
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


#include "cppa/event_based_actor.hpp"

namespace cppa {

void event_based_actor::become_void()
{
    while (!m_loop_stack.empty()) m_loop_stack.pop();
}

void event_based_actor::do_become(behavior* bhvr)
{
    if (bhvr->is_left())
    {
        do_become(&(bhvr->left()), false);
    }
    else
    {
        do_become(&(bhvr->right()), false);
    }
}

void event_based_actor::do_become(invoke_rules* bhvr, bool has_ownership)
{
    become_void();
    reset_timeout();
    m_loop_stack.push(stack_element(bhvr, has_ownership));
}

void event_based_actor::do_become(timed_invoke_rules* bhvr, bool has_ownership)
{
    become_void();
    request_timeout(bhvr->timeout());
    m_loop_stack.push(stack_element(bhvr, has_ownership));
}

} // namespace cppa