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


#ifndef BOOST_ACTOR_ACTOR_ADDR_HPP
#define BOOST_ACTOR_ACTOR_ADDR_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>

#include "boost/intrusive_ptr.hpp"
#include "boost/actor/abstract_actor.hpp"

#include "boost/actor/detail/comparable.hpp"

namespace boost {
namespace actor {

class actor;
class local_actor;
class actor_namespace;

namespace detail { class raw_access; }

/**
 * @brief Identifies an invalid {@link actor_addr}.
 * @relates actor_addr
 */
struct invalid_actor_addr_t { constexpr invalid_actor_addr_t() { } };

constexpr invalid_actor_addr_t invalid_actor_addr = invalid_actor_addr_t{};

/**
 * @brief Stores the address of typed as well as untyped actors.
 */
class actor_addr : detail::comparable<actor_addr>
                 , detail::comparable<actor_addr, abstract_actor*>
                 , detail::comparable<actor_addr, abstract_actor_ptr> {

    friend class actor;
    friend class abstract_actor;
    friend class detail::raw_access;

 public:

    actor_addr() = default;

    actor_addr(actor_addr&&) = default;

    actor_addr(const actor_addr&) = default;

    actor_addr& operator=(actor_addr&&) = default;

    actor_addr& operator=(const actor_addr&) = default;

    actor_addr(const invalid_actor_addr_t&);

    actor_addr operator=(const invalid_actor_addr_t&);

    inline explicit operator bool() const {
        return static_cast<bool>(m_ptr);
    }

    inline bool operator!() const {
        return !m_ptr;
    }

    intptr_t compare(const actor_addr& other) const;

    intptr_t compare(const abstract_actor* other) const;

    inline intptr_t compare(const abstract_actor_ptr& other) const {
        return compare(other.get());
    }

    actor_id id() const;

    const node_id& node() const;

    /**
     * @brief Returns whether this is an address of a
     *        remote actor.
     */
    bool is_remote() const;

    std::set<std::string> interface() const;

 private:

    explicit actor_addr(abstract_actor*);

    abstract_actor_ptr m_ptr;

};

} // namespace actor
} // namespace boost

// allow actor_addr to be used in hash maps
namespace std {
template<>
struct hash<boost::actor::actor_addr> {
    inline size_t operator()(const boost::actor::actor_addr& ref) const {
        return static_cast<size_t>(ref.id());
    }
};
} // namespace std


#endif // BOOST_ACTOR_ACTOR_ADDR_HPP
