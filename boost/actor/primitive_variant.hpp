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


#ifndef BOOST_ACTOR_PRIMITIVE_VARIANT_HPP
#define BOOST_ACTOR_PRIMITIVE_VARIANT_HPP

#include <new>
#include <cstdint>
#include <typeinfo>
#include <stdexcept>
#include <type_traits>

#include "boost/none.hpp"
#include "boost/variant.hpp"

#include "boost/actor/atom.hpp"

namespace boost {
namespace actor {

typedef variant<
            int8_t,
            int16_t,
            int32_t,
            int64_t,
            uint8_t,
            uint16_t,
            uint32_t,
            uint64_t,
            float,
            double,
            long double,
            std::string,
            std::u16string,
            std::u32string,
            atom_value
        >
        primitive_variant;

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_PRIMITIVE_VARIANT_HPP
