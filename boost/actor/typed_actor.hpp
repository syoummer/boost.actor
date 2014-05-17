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


#ifndef BOOST_ACTOR_TYPED_ACTOR_HPP
#define BOOST_ACTOR_TYPED_ACTOR_HPP

#include "boost/actor/actor_addr.hpp"
#include "boost/actor/replies_to.hpp"
#include "boost/intrusive_ptr.hpp"
#include "boost/actor/abstract_actor.hpp"
#include "boost/actor/typed_behavior.hpp"

namespace boost {
namespace actor {

class actor_addr;
class local_actor;

struct invalid_actor_addr_t;

template<typename... Rs>
class typed_event_based_actor;

namespace detail {

class raw_access;

} // namespace detail

/**
 * @brief Identifies a strongly typed actor.
 * @tparam Rs Interface as @p replies_to<...>::with<...> parameter pack.
 */
template<typename... Rs>
class typed_actor : detail::comparable<typed_actor<Rs...>>
                  , detail::comparable<typed_actor<Rs...>, actor_addr>
                  , detail::comparable<typed_actor<Rs...>, invalid_actor_addr_t> {

    friend class local_actor;

    friend class detail::raw_access;

    // friend with all possible instantiations
    template<typename...>
    friend class typed_actor;

 public:

    /**
     * @brief Identifies the behavior type actors of this kind use
     *        for their behavior stack.
     */
    typedef typed_behavior<Rs...> behavior_type;

    /**
     * @brief Identifies pointers to instances of this kind of actor.
     */
    typedef typed_event_based_actor<Rs...>* pointer;

    /**
     * @brief Identifies the base class for this kind of actor.
     */
    typedef typed_event_based_actor<Rs...> base;

    /**
     * @brief Stores the interface of the actor as type list.
     */
    typedef detail::type_list<Rs...> interface;

    typed_actor() = default;
    typed_actor(typed_actor&&) = default;
    typed_actor(const typed_actor&) = default;
    typed_actor& operator=(typed_actor&&) = default;
    typed_actor& operator=(const typed_actor&) = default;

    template<typename... OtherRs>
    typed_actor(const typed_actor<OtherRs...>& other) {
        set(std::move(other));
    }

    template<typename... OtherRs>
    typed_actor& operator=(const typed_actor<OtherRs...>& other) {
        set(std::move(other));
        return *this;
    }

    template<class Impl>
    typed_actor(intrusive_ptr<Impl> other) {
        set(other);
    }

    pointer operator->() const {
        return static_cast<pointer>(m_ptr.get());
    }

    base& operator*() const {
        return static_cast<base&>(*m_ptr.get());
    }

    /**
     * @brief Queries the address of the stored actor.
     */
    actor_addr address() const {
        return m_ptr ? m_ptr->address() : actor_addr{};
    }

    inline intptr_t compare(const actor_addr& rhs) const {
        return address().compare(rhs);
    }

    inline intptr_t compare(const typed_actor& other) const {
        return compare(other.address());
    }

    inline intptr_t compare(const invalid_actor_addr_t&) const {
        return m_ptr ? 1 : 0;
    }

    static std::set<std::string> get_interface() {
        return {detail::to_uniform_name<Rs>()...};
    }

    explicit operator bool() const {
        return static_cast<bool>(m_ptr);
    }

    inline bool operator!() const {
        return !m_ptr;
    }

 private:

    typed_actor(abstract_actor* ptr) : m_ptr(ptr) { }

    template<class ListA, class ListB>
    inline void check_signatures() {
        static_assert(detail::tl_is_strict_subset<ListA, ListB>::value,
                      "'this' must be a strict subset of 'other'");
    }

    template<typename... OtherRs>
    inline void set(const typed_actor<OtherRs...>& other) {
        check_signatures<interface, detail::type_list<OtherRs...>>();
        m_ptr = other.m_ptr;
    }

    template<class Impl>
    inline void set(intrusive_ptr<Impl>& other) {
        check_signatures<interface, typename Impl::signatures>();
        m_ptr = std::move(other);
    }

    abstract_actor_ptr m_ptr;

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_TYPED_ACTOR_HPP
