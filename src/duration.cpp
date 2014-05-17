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


#include <sstream>

#include "boost/actor/duration.hpp"

namespace boost {
namespace actor {

namespace {

inline std::uint64_t ui64_val(const duration& d) {
    return static_cast<std::uint64_t>(d.unit) * d.count;
}

} // namespace <anonmyous>

bool operator==(const duration& lhs, const duration& rhs) {
    return (lhs.unit == rhs.unit ? lhs.count == rhs.count
                                 : ui64_val(lhs) == ui64_val(rhs));
}

std::string duration::to_string() const {
    if (unit == time_unit::invalid) return "-invalid-";
    std::ostringstream oss;
    oss << count;
    switch (unit) {
        case time_unit::invalid: oss << "?"; break;
        case time_unit::seconds: oss << "s"; break;
        case time_unit::milliseconds: oss << "ms"; break;
        case time_unit::microseconds: oss << "us"; break;
    }
    return oss.str();
}

} // namespace actor
} // namespace boost
