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
            std::int8_t,
            std::int16_t,
            std::int32_t,
            std::int64_t,
            std::uint8_t,
            std::uint16_t,
            std::uint32_t,
            std::uint64_t,
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
