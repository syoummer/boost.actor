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


#ifndef BOOST_ACTOR_GROUP_HPP
#define BOOST_ACTOR_GROUP_HPP

#include "boost/intrusive_ptr.hpp"

#include "boost/actor/fwd.hpp"
#include "boost/actor/abstract_group.hpp"

#include "boost/actor/detail/comparable.hpp"
#include "boost/actor/detail/type_traits.hpp"

namespace boost {
namespace actor {

class channel;
class message;

struct invalid_group_t { constexpr invalid_group_t() { } };

/**
 * @brief Identifies an invalid {@link group}.
 * @relates group
 */
constexpr invalid_group_t invalid_group = invalid_group_t{};

class group : detail::comparable<group>
            , detail::comparable<group, invalid_group_t> {

    template<typename T, typename U>
    friend T actor_cast(const U&);

 public:

    group() = default;

    group(group&&) = default;

    group(const group&) = default;

    group(const invalid_group_t&);

    group& operator=(group&&) = default;

    group& operator=(const group&) = default;

    group& operator=(const invalid_group_t&);

    group(intrusive_ptr<abstract_group> ptr);

    inline explicit operator bool() const {
        return static_cast<bool>(m_ptr);
    }

    inline bool operator!() const {
        return !static_cast<bool>(m_ptr);
    }

    /**
     * @brief Returns a handle that grants access to
     *        actor operations such as enqueue.
     */
    inline abstract_group* operator->() const {
        return m_ptr.get();
    }

    inline abstract_group& operator*() const {
        return *m_ptr;
    }

    intptr_t compare(const group& other) const;

    inline intptr_t compare(const invalid_actor_t&) const {
        return m_ptr ? 1 : 0;
    }

    /**
     * @brief Get a pointer to the group associated with
     *        @p group_identifier from the module @p module_name.
     * @threadsafe
     */
    static group get(const std::string& module_name,
                     const std::string& group_identifier);

    /**
     * @brief Returns an anonymous group.
     *
     * Each calls to this member function returns a new instance
     * of an anonymous group. Anonymous groups can be used whenever
     * a set of actors wants to communicate using an exclusive channel.
     */
    static group anonymous();

    /**
     * @brief Add a new group module to the group management.
     * @threadsafe
     */
    static void add_module(abstract_group::unique_module_ptr);

    /**
     * @brief Returns the module associated with @p module_name.
     * @threadsafe
     */
    static abstract_group::module_ptr get_module(const std::string& module_name);


 private:

    inline abstract_group* get() const {
        return m_ptr.get();
    }

    abstract_group_ptr m_ptr;

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_GROUP_HPP
