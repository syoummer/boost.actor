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


#ifndef BOOST_ACTOR_ABSTRACT_TUPLE_HPP
#define BOOST_ACTOR_ABSTRACT_TUPLE_HPP

#include <string>
#include <iterator>
#include <typeinfo>

#include "boost/intrusive_ptr.hpp"

#include "boost/actor/config.hpp"
#include "boost/actor/ref_counted.hpp"
#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/util/type_list.hpp"

#include "boost/actor/detail/tuple_iterator.hpp"

namespace boost {
namespace actor {
namespace detail {

class abstract_tuple : public ref_counted {

 public:

    abstract_tuple(bool dynamically_typed);
    abstract_tuple(const abstract_tuple& other);

    // mutators
    virtual void* mutable_at(size_t pos) = 0;
    virtual void* mutable_native_data();

    // accessors
    virtual size_t size() const = 0;
    virtual abstract_tuple* copy() const = 0;
    virtual const void* at(size_t pos) const = 0;
    virtual const uniform_type_info* type_at(size_t pos) const = 0;
    virtual const std::string* tuple_type_names() const = 0;

    // returns either tdata<...> object or nullptr (default) if tuple
    // is not a 'native' implementation
    virtual const void* native_data() const;

    // Identifies the type of the implementation.
    // A statically typed tuple implementation can use some optimizations,
    // e.g., "impl_type() == statically_typed" implies that type_token()
    // identifies all possible instances of a given tuple implementation
    inline bool dynamically_typed() const { return m_is_dynamic; }

    // uniquely identifies this category (element types) of messages
    // override this member function only if impl_type() == statically_typed
    // (default returns &typeid(void))
    virtual const std::type_info* type_token() const;

    bool equals(const abstract_tuple& other) const;

    typedef tuple_iterator<abstract_tuple> const_iterator;

    inline const_iterator  begin() const { return {this}; }
    inline const_iterator cbegin() const { return {this}; }

    inline const_iterator  end() const { return {this, size()}; }
    inline const_iterator cend() const { return {this, size()}; }

    class ptr {

     public:

        ptr() = default;
        ptr(ptr&&) = default;
        ptr(const ptr&) = default;
        ptr& operator=(ptr&&) = default;
        ptr& operator=(const ptr&) = default;

        inline explicit ptr(abstract_tuple* p) : m_ptr(p) { }

        inline void detach() { static_cast<void>(get_detached()); }

        inline abstract_tuple* operator->() { return get_detached(); }
        inline abstract_tuple& operator*() { return *get_detached(); }
        inline const abstract_tuple* operator->() const { return m_ptr.get(); }
        inline const abstract_tuple& operator*() const { return *m_ptr.get(); }
        inline void swap(ptr& other) { m_ptr.swap(other.m_ptr); }
        inline void reset(abstract_tuple* p = nullptr) { m_ptr.reset(p); }

        inline explicit operator bool() const {
            return static_cast<bool>(m_ptr);
        }

     private:

        abstract_tuple* get_detached();

        intrusive_ptr<abstract_tuple> m_ptr;

    };

 private:

    bool m_is_dynamic;

};

struct full_eq_type {
    constexpr full_eq_type() { }
    template<class Tuple>
    inline bool operator()(const tuple_iterator<Tuple>& lhs,
                           const tuple_iterator<Tuple>& rhs) const {
        return    lhs.type() == rhs.type()
               && lhs.type()->equals(lhs.value(), rhs.value());
    }
};

struct types_only_eq_type {
    constexpr types_only_eq_type() { }
    template<class Tuple>
    inline bool operator()(const tuple_iterator<Tuple>& lhs,
                           const uniform_type_info* rhs     ) const {
        return lhs.type() == rhs;
    }
    template<class Tuple>
    inline bool operator()(const uniform_type_info* lhs,
                           const tuple_iterator<Tuple>& rhs) const {
        return lhs == rhs.type();
    }
};

namespace {
constexpr full_eq_type full_eq;
constexpr types_only_eq_type types_only_eq;
} // namespace <anonymous>

std::string get_tuple_type_names(const detail::abstract_tuple&);

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_ABSTRACT_TUPLE_HPP
