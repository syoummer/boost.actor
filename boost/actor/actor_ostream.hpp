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


#ifndef BOOST_ACTOR_ACTOR_OSTREAM_HPP
#define BOOST_ACTOR_ACTOR_OSTREAM_HPP

#include "boost/actor/actor.hpp"
#include "boost/actor/any_tuple.hpp"
#include "boost/actor/to_string.hpp"

namespace boost {
namespace actor {

class local_actor;
class scoped_actor;

class actor_ostream {

 public:

    typedef actor_ostream& (*fun_type)(actor_ostream&);

    actor_ostream(actor_ostream&&) = default;
    actor_ostream(const actor_ostream&) = default;

    actor_ostream& operator=(actor_ostream&&) = default;
    actor_ostream& operator=(const actor_ostream&) = default;

    explicit actor_ostream(actor self);

    actor_ostream& write(std::string arg);

    actor_ostream& flush();

    inline actor_ostream& operator<<(std::string arg) {
        return write(std::move(arg));
    }

    inline actor_ostream& operator<<(const any_tuple& arg) {
        return write(boost::actor::to_string(arg));
    }

    // disambiguate between conversion to string and to any_tuple
    inline actor_ostream& operator<<(const char* arg) {
        return *this << std::string{arg};
    }

    template<typename T>
    inline typename std::enable_if<
           !std::is_convertible<T, std::string>::value
        && !std::is_convertible<T, any_tuple>::value,
        actor_ostream&
    >::type
    operator<<(T&& arg) {
        return write(std::to_string(std::forward<T>(arg)));
    }

    inline actor_ostream& operator<<(actor_ostream::fun_type f) {
        return f(*this);
    }

 private:

    actor m_self;
    actor m_printer;

};

inline actor_ostream aout(actor self) {
    return actor_ostream{self};
}

} // namespace actor
} // namespace boost

namespace std {
// provide convenience overlaods for aout; implemented in logging.cpp
boost::actor::actor_ostream& endl(boost::actor::actor_ostream& o);
boost::actor::actor_ostream& flush(boost::actor::actor_ostream& o);
} // namespace std

#endif // BOOST_ACTOR_ACTOR_OSTREAM_HPP
