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


#ifndef EVENT_HPP
#define EVENT_HPP

namespace boost {
namespace actor {
namespace io {

typedef int event_bitmask;

namespace event { namespace {

constexpr event_bitmask none  = 0x00;
constexpr event_bitmask read  = 0x01;
constexpr event_bitmask write = 0x02;
constexpr event_bitmask both  = 0x03;
constexpr event_bitmask error = 0x04;

} } // namespace <anonymous>::event

template<unsigned InputEvent, unsigned OutputEvent, unsigned ErrorEvent>
inline event_bitmask from_int_bitmask(unsigned mask) {
    event_bitmask result = event::none;
    // read/write as long as possible
    if (mask & InputEvent) result = event::read;
    if (mask & OutputEvent) result |= event::write;
    if (result == event::none && mask & ErrorEvent) result = event::error;
    return result;
}

} // namespace io
} // namespace actor
} // namespace boost

#endif // EVENT_HPP
