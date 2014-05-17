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


#ifndef BOOST_ACTOR_SWAP_BYTES_HPP
#define BOOST_ACTOR_SWAP_BYTES_HPP

#include <cstddef>

namespace boost {
namespace actor {
namespace detail {

template<typename T>
struct byte_access {
    union {
        T value;
        unsigned char bytes[sizeof(T)];
    };
    inline byte_access(T val = 0) : value(val) { }
};

template<size_t SizeOfT, typename T>
struct byte_swapper {
    static T _(byte_access<T> from) {
        byte_access<T> to;
        auto i = SizeOfT - 1;
        for (size_t j = 0 ; j < SizeOfT ; --i, ++j) {
            to.bytes[i] = from.bytes[j];
        }
        return to.value;
    }
};

template<typename T>
struct byte_swapper<1, T> {
    inline static T _(T what) { return what; }
};

template<typename T>
inline T swap_bytes(T what) {
    return byte_swapper<sizeof(T), T>::_(what);
}

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_SWAP_BYTES_HPP
