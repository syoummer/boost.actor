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


#ifndef BOOST_ACTOR_ABSTRACT_TUPLE_HPP
#define BOOST_ACTOR_ABSTRACT_TUPLE_HPP

#include <string>
#include <iterator>
#include <typeinfo>

#include "boost/intrusive_ptr.hpp"

#include "boost/actor/config.hpp"
#include "boost/actor/ref_counted.hpp"
#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/detail/type_list.hpp"

#include "boost/actor/detail/message_iterator.hpp"

namespace boost {
namespace actor {
namespace detail {

class message_data : public ref_counted {

 public:

    message_data(bool dynamically_typed);
    message_data(const message_data& other);

    // mutators
    virtual void* mutable_at(size_t pos) = 0;
    virtual void* mutable_native_data();

    // accessors
    virtual size_t size() const = 0;
    virtual message_data* copy() const = 0;
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

    bool equals(const message_data& other) const;

    typedef message_iterator<message_data> const_iterator;

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

        inline explicit ptr(message_data* p) : m_ptr(p) { }

        inline void detach() { static_cast<void>(get_detached()); }

        inline message_data* operator->() { return get_detached(); }
        inline message_data& operator*() { return *get_detached(); }
        inline const message_data* operator->() const { return m_ptr.get(); }
        inline const message_data& operator*() const { return *m_ptr.get(); }
        inline void swap(ptr& other) { m_ptr.swap(other.m_ptr); }
        inline void reset(message_data* p = nullptr) { m_ptr.reset(p); }

        inline explicit operator bool() const {
            return static_cast<bool>(m_ptr);
        }

     private:

        message_data* get_detached();

        intrusive_ptr<message_data> m_ptr;

    };

 private:

    bool m_is_dynamic;

};

struct full_eq_type {
    constexpr full_eq_type() { }
    template<class Tuple>
    inline bool operator()(const message_iterator<Tuple>& lhs,
                           const message_iterator<Tuple>& rhs) const {
        return    lhs.type() == rhs.type()
               && lhs.type()->equals(lhs.value(), rhs.value());
    }
};

struct types_only_eq_type {
    constexpr types_only_eq_type() { }
    template<class Tuple>
    inline bool operator()(const message_iterator<Tuple>& lhs,
                           const uniform_type_info* rhs     ) const {
        return lhs.type() == rhs;
    }
    template<class Tuple>
    inline bool operator()(const uniform_type_info* lhs,
                           const message_iterator<Tuple>& rhs) const {
        return lhs == rhs.type();
    }
};

namespace {
constexpr full_eq_type full_eq;
constexpr types_only_eq_type types_only_eq;
} // namespace <anonymous>

std::string get_tuple_type_names(const detail::message_data&);

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_ABSTRACT_TUPLE_HPP
