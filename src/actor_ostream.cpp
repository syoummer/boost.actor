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


#include "boost/actor/cppa.hpp"
#include "boost/actor/scheduler.hpp"
#include "boost/actor/singletons.hpp"
#include "boost/actor/local_actor.hpp"
#include "boost/actor/scoped_actor.hpp"
#include "boost/actor/actor_ostream.hpp"

namespace boost {
namespace actor {

actor_ostream::actor_ostream(actor self) : m_self(std::move(self)) {
    m_printer = get_scheduling_coordinator()->printer();
}

actor_ostream& actor_ostream::write(std::string arg) {
    send_as(m_self, m_printer, atom("add"), std::move(arg));
    return *this;
}

actor_ostream& actor_ostream::flush() {
    send_as(m_self, m_printer, atom("flush"));
    return *this;
}

} // namespace actor
} // namespace boost

namespace std {

boost::actor::actor_ostream& endl(boost::actor::actor_ostream& o) {
    return o.write("\n");
}

boost::actor::actor_ostream& flush(boost::actor::actor_ostream& o) {
    return o.flush();
}

} // namespace std
