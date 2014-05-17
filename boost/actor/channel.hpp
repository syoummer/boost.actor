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


#ifndef BOOST_ACTOR_CHANNEL_HPP
#define BOOST_ACTOR_CHANNEL_HPP

#include <cstddef>
#include <type_traits>

#include "boost/intrusive_ptr.hpp"
#include "boost/actor/abstract_channel.hpp"

#include "boost/actor/detail/comparable.hpp"

namespace boost {
namespace actor {

class actor;
class group;
class execution_unit;

struct invalid_actor_t;
struct invalid_group_t;

namespace detail { class raw_access; }

/**
 * @brief A handle to instances of {@link abstract_channel}.
 */
class channel : detail::comparable<channel>
              , detail::comparable<channel, actor>
              , detail::comparable<channel, abstract_channel*> {

    friend class detail::raw_access;

 public:

    channel() = default;

    channel(const actor&);

    channel(const group&);

    channel(const invalid_actor_t&);

    channel(const invalid_group_t&);

    template<typename T>
    channel(intrusive_ptr<T> ptr,
            typename std::enable_if<
                std::is_base_of<abstract_channel, T>::value
            >::type* = 0)
        : m_ptr(ptr) { }

    channel(abstract_channel* ptr);

    inline explicit operator bool() const {
        return static_cast<bool>(m_ptr);
    }

    inline bool operator!() const {
        return !m_ptr;
    }

    inline abstract_channel* operator->() const {
        return m_ptr.get();
    }

    inline abstract_channel& operator*() const {
        return *m_ptr;
    }

    intptr_t compare(const channel& other) const;

    intptr_t compare(const actor& other) const;

    intptr_t compare(const abstract_channel* other) const;

    static intptr_t compare(const abstract_channel* lhs,
                            const abstract_channel* rhs);

 private:

    intrusive_ptr<abstract_channel> m_ptr;

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_CHANNEL_HPP
