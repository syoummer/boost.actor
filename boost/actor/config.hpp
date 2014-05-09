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


#ifndef BOOST_ACTOR_CONFIG_HPP
#define BOOST_ACTOR_CONFIG_HPP

#include "boost/config.hpp"

// Config pararameters defined by the build system (usually CMake):
//
// BOOST_ACTOR_DEBUG_MODE:
//     - check requirements at runtime
//
// BOOST_ACTOR_LOG_LEVEL:
//     - denotes the amount of logging, ranging from error messages only (0)
//       to complete traces (4)

// sets BOOST_ACTOR_DEPRECATED, BOOST_ACTOR_ANNOTATE_FALLTHROUGH,
// BOOST_ACTOR_PUSH_WARNINGS and BOOST_ACTOR_POP_WARNINGS
#ifdef BOOST_CLANG
#  define BOOST_ACTOR_PUSH_WARNINGS                                                   \
        _Pragma("clang diagnostic push")                                       \
        _Pragma("clang diagnostic ignored \"-Wall\"")                          \
        _Pragma("clang diagnostic ignored \"-Wextra\"")                        \
        _Pragma("clang diagnostic ignored \"-Werror\"")                        \
        _Pragma("clang diagnostic ignored \"-Wdeprecated\"")                   \
        _Pragma("clang diagnostic ignored \"-Wdisabled-macro-expansion\"")     \
        _Pragma("clang diagnostic ignored \"-Wextra-semi\"")                   \
        _Pragma("clang diagnostic ignored \"-Wdocumentation\"")                \
        _Pragma("clang diagnostic ignored \"-Wweak-vtables\"")                 \
        _Pragma("clang diagnostic ignored \"-Wunused-parameter\"")             \
        _Pragma("clang diagnostic ignored \"-Wswitch-enum\"")                  \
        _Pragma("clang diagnostic ignored \"-Wshadow\"")                       \
        _Pragma("clang diagnostic ignored \"-Wconversion\"")                   \
        _Pragma("clang diagnostic ignored \"-Wcast-align\"")                   \
        _Pragma("clang diagnostic ignored \"-Wundef\"")
#  define BOOST_ACTOR_POP_WARNINGS                                                    \
        _Pragma("clang diagnostic pop")
#  define BOOST_ACTOR_ANNOTATE_FALLTHROUGH [[clang::fallthrough]]
#else
#  define BOOST_ACTOR_PUSH_WARNINGS
#  define BOOST_ACTOR_POP_WARNINGS
#  define BOOST_ACTOR_ANNOTATE_FALLTHROUGH static_cast<void>(0)
#endif

// detect OS
#if defined(__APPLE__)
#  define BOOST_ACTOR_MACOS
#elif defined(__linux__)
#  define BOOST_ACTOR_LINUX
#   include <linux/version.h>
#   if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,16)
#     define BOOST_ACTOR_POLL_IMPL
#   endif
#elif defined(WIN32)
#  define BOOST_ACTOR_WINDOWS
#else
#  error Platform and/or compiler not supportet
#endif

#include <cstdio>
#include <cstdlib>

#ifdef BOOST_ACTOR_DEBUG_MODE
#   define BOOST_ACTOR_REQUIRE__(stmt, file, line)                             \
        printf("%s:%u: requirement failed '%s'\n", file, line, stmt);          \
        abort()
#   define BOOST_ACTOR_REQUIRE(stmt)                                           \
        if (static_cast<bool>(stmt) == false) {                                \
            BOOST_ACTOR_REQUIRE__(#stmt, __FILE__, __LINE__);                  \
        }((void) 0)
#else
#   define BOOST_ACTOR_REQUIRE(unused) static_cast<void>(0)
#endif

#define BOOST_ACTOR_CRITICAL__(error, file, line) {                            \
        printf("%s:%u: critical error: '%s'\n", file, line, error);            \
        exit(7);                                                               \
    } ((void) 0)

#define BOOST_ACTOR_CRITICAL(error) BOOST_ACTOR_CRITICAL__(error, __FILE__, __LINE__)

#endif // BOOST_ACTOR_CONFIG_HPP
