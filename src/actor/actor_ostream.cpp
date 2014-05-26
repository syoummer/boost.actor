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


#include "boost/actor/all.hpp"
#include "boost/actor/scheduler.hpp"
#include "boost/actor/local_actor.hpp"
#include "boost/actor/scoped_actor.hpp"
#include "boost/actor/actor_ostream.hpp"

#include "boost/actor/detail/singletons.hpp"

namespace boost {
namespace actor {

actor_ostream::actor_ostream(actor self) : m_self(std::move(self)) {
    m_printer = detail::singletons::get_scheduling_coordinator()->printer();
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
