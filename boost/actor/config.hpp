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

// Config pararameters defined by the build system (usually CMake):
//
// BOOST_ACTOR_STANDALONE_BUILD:
//     - builds libcppa without Boost
//
// BOOST_ACTOR_DISABLE_CONTEXT_SWITCHING:
//     - disables context switching even if Boost is available
//
// BOOST_ACTOR_DEBUG_MODE:
//     - check requirements at runtime
//
// BOOST_ACTOR_LOG_LEVEL:
//     - denotes the amount of logging, ranging from error messages only (0)
//       to complete traces (4)
//
// BOOST_ACTOR_OPENCL:
//     - enables optional OpenCL module

/**
 * @brief Denotes the libcppa version in the format {MAJOR}{MINOR}{PATCH},
 *        whereas each number is a two-digit decimal number without
 *        leading zeros (e.g. 900 is version 0.9.0).
 */
#define BOOST_ACTOR_VERSION 900

#define BOOST_ACTOR_MAJOR_VERSION (BOOST_ACTOR_VERSION / 100000)
#define BOOST_ACTOR_MINOR_VERSION ((BOOST_ACTOR_VERSION / 100) % 1000)
#define BOOST_ACTOR_PATCH_VERSION (BOOST_ACTOR_VERSION % 100)

// sets BOOST_ACTOR_DEPRECATED, BOOST_ACTOR_ANNOTATE_FALLTHROUGH,
// BOOST_ACTOR_PUSH_WARNINGS and BOOST_ACTOR_POP_WARNINGS
#if defined(__clang__)
#  define BOOST_ACTOR_CLANG
#  define BOOST_ACTOR_DEPRECATED __attribute__((__deprecated__))
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
#elif defined(__GNUC__)
#  define BOOST_ACTOR_GCC
#  define BOOST_ACTOR_DEPRECATED __attribute__((__deprecated__))
#  define BOOST_ACTOR_PUSH_WARNINGS
#  define BOOST_ACTOR_POP_WARNINGS
#  define BOOST_ACTOR_ANNOTATE_FALLTHROUGH static_cast<void>(0)
#elif defined(_MSC_VER)
#  define BOOST_ACTOR_DEPRECATED __declspec(deprecated)
#  define BOOST_ACTOR_PUSH_WARNINGS
#  define BOOST_ACTOR_POP_WARNINGS
#  define BOOST_ACTOR_ANNOTATE_FALLTHROUGH static_cast<void>(0)
#else
#  define BOOST_ACTOR_DEPRECATED
#  define BOOST_ACTOR_PUSH_WARNINGS
#  define BOOST_ACTOR_POP_WARNINGS
#  define BOOST_ACTOR_ANNOTATE_FALLTHROUGH static_cast<void>(0)
#endif

// detect OS
#if defined(__APPLE__)
#  define BOOST_ACTOR_MACOS
#  ifndef _GLIBCXX_HAS_GTHREADS
#    define _GLIBCXX_HAS_GTHREADS
#  endif
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

#include <memory>
#include <cstdio>
#include <cstdlib>

// import backtrace and backtrace_symbols_fd into cppa::detail
#ifdef BOOST_ACTOR_WINDOWS
#include "boost/actor/detail/execinfo_windows.hpp"
#else
#include <execinfo.h>
namespace boost {
namespace actor {
namespace detail {
using ::backtrace;
using ::backtrace_symbols_fd;
} // namespace detail
} // namespace actor
} // namespace boost
#endif

//
#ifdef BOOST_ACTOR_DEBUG_MODE
#   define BOOST_ACTOR_REQUIRE__(stmt, file, line)                                    \
        printf("%s:%u: requirement failed '%s'\n", file, line, stmt);          \
        {                                                                      \
            void* array[10];                                                   \
            auto cppa_bt_size = ::cppa::detail::backtrace(array, 10);          \
            ::cppa::detail::backtrace_symbols_fd(array, cppa_bt_size, 2);      \
        }                                                                      \
        abort()
#   define BOOST_ACTOR_REQUIRE(stmt)                                                  \
        if ((stmt) == false) {                                                 \
            BOOST_ACTOR_REQUIRE__(#stmt, __FILE__, __LINE__);                         \
        }((void) 0)
#else
#   define BOOST_ACTOR_REQUIRE(unused) static_cast<void>(0)
#endif

#define BOOST_ACTOR_CRITICAL__(error, file, line) {                                   \
        printf("%s:%u: critical error: '%s'\n", file, line, error);            \
        exit(7);                                                               \
    } ((void) 0)

#define BOOST_ACTOR_CRITICAL(error) BOOST_ACTOR_CRITICAL__(error, __FILE__, __LINE__)

#ifdef BOOST_ACTOR_WINDOWS
#   include <w32api.h>
#   undef _WIN32_WINNT
#   undef WINVER
#   define _WIN32_WINNT WindowsVista
#   define WINVER WindowsVista
#   include <ws2tcpip.h>
#   include <winsock2.h>
    // remove interface which is defined in rpc.h in files included by
    // windows.h as it clashes with name used in own code
#   undef interface
#else
#   include <unistd.h>
#   include <errno.h>
#endif

namespace boost {
namespace actor {

/**
 * @brief An alternative for the 'missing' @p std::make_unqiue.
 */
template<typename T, typename... Args>
std::unique_ptr<T> create_unique(Args&&... args) {
    return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
}

// platform-dependent types for sockets and some utility functions
#ifdef BOOST_ACTOR_WINDOWS
    typedef SOCKET native_socket_type;
    typedef const char* setsockopt_ptr;
    typedef const char* socket_send_ptr;
    typedef char* socket_recv_ptr;
    typedef int socklen_t;
    constexpr SOCKET invalid_socket = INVALID_SOCKET;
    inline int last_socket_error() { return WSAGetLastError(); }
    inline bool would_block_or_temporarily_unavailable(int errcode) {
        return errcode == WSAEWOULDBLOCK || errcode == WSATRY_AGAIN;
    }
#else
    typedef int native_socket_type;
    typedef const void* setsockopt_ptr;
    typedef const void* socket_send_ptr;
    typedef void* socket_recv_ptr;
    constexpr int invalid_socket = -1;
    inline void closesocket(native_socket_type fd) { close(fd); }
    inline int last_socket_error() { return errno; }
    inline bool would_block_or_temporarily_unavailable(int errcode) {
        return errcode == EAGAIN || errcode == EWOULDBLOCK;
    }
#endif

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_CONFIG_HPP
