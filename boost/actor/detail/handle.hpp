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


#ifndef BOOST_ACTOR_DETAIL_HANDLE_HPP
#define BOOST_ACTOR_DETAIL_HANDLE_HPP

#include "boost/actor/detail/comparable.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename Subtype>
class handle : detail::comparable<Subtype> {

 public:

    inline handle() : m_id{-1} { }

    handle(const Subtype& other) {
        m_id = other.id();
    }

    handle(const handle& other) : m_id(other.m_id) { }

    Subtype& operator=(const handle& other) {
        m_id = other.id();
        return *static_cast<Subtype*>(this);
    }

    inline int id() const {
        return m_id;
    }

    inline int compare(const Subtype& other) const {
        return m_id - other.id();
    }

    inline bool invalid() const {
        return m_id == -1;
    }

    static inline Subtype from_int(int id) {
        return {id};
    }


 protected:

    inline handle(int handle_id) : m_id{handle_id} { }

 private:

    int m_id;

};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_HANDLE_HPP
