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


#include "boost/actor/primitive_variant.hpp"

namespace boost {
namespace actor {

void primitive_variant_init(primitive_variant& what, primitive_type ptype) {
    switch (ptype) {
        case pt_int8:           what = int8_t{0};           break;
        case pt_int16:          what = int16_t{0};          break;
        case pt_int32:          what = int32_t{0};          break;
        case pt_int64:          what = int64_t{0};          break;
        case pt_uint8:          what = uint8_t{0};          break;
        case pt_uint16:         what = uint16_t{0};         break;
        case pt_uint32:         what = uint32_t{0};         break;
        case pt_uint64:         what = uint64_t{0};         break;
        case pt_float:          what = 0.f;                 break;
        case pt_double:         what = 0.;                  break;
        case pt_long_double:    what = static_cast<long double>(0.);    break;
        case pt_u8string:       what = std::string{};       break;
        case pt_u16string:      what = std::u16string{};    break;
        case pt_u32string:      what = std::u32string{};    break;
        case pt_atom:           what = atom("");            break;
    }
}

} // namespace actor
} // namespace boost
