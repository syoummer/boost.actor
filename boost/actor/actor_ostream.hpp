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


#ifndef BOOST_ACTOR_ACTOR_OSTREAM_HPP
#define BOOST_ACTOR_ACTOR_OSTREAM_HPP

#include "boost/actor/actor.hpp"
#include "boost/actor/message.hpp"
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

    inline actor_ostream& operator<<(const message& arg) {
        return write(boost::actor::to_string(arg));
    }

    // disambiguate between conversion to string and to message
    inline actor_ostream& operator<<(const char* arg) {
        return *this << std::string{arg};
    }

    template<typename T>
    inline typename std::enable_if<
           !std::is_convertible<T, std::string>::value
        && !std::is_convertible<T, message>::value,
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
