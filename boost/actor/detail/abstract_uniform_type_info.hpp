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


#ifndef BOOST_ACTOR_ABSTRACT_UNIFORM_TYPE_INFO_HPP
#define BOOST_ACTOR_ABSTRACT_UNIFORM_TYPE_INFO_HPP

#include "boost/actor/message.hpp"
#include "boost/actor/deserializer.hpp"
#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/detail/type_traits.hpp"

#include "boost/actor/detail/to_uniform_name.hpp"
#include "boost/actor/detail/uniform_type_info_map.hpp"

namespace boost {
namespace actor {
namespace detail {

/**
 * @brief Implements all pure virtual functions of {@link uniform_type_info}
 *        except serialize() and deserialize().
 */
template<typename T>
class abstract_uniform_type_info : public uniform_type_info {

 public:

    bool equal_to(const std::type_info& tinfo) const override {
        return typeid(T) == tinfo;
    }

    const char* name() const {
        return m_name.c_str();
    }

    message as_message(void* instance) const override {
        return make_message(deref(instance));
    }

    bool equals(const void* lhs, const void* rhs) const override {
        return eq(deref(lhs), deref(rhs));
    }

    uniform_value create(const uniform_value& other) const override {
        return create_impl<T>(other);
    }

 protected:

    abstract_uniform_type_info() {
        auto uname = detail::to_uniform_name<T>();
        auto cname = detail::mapped_name_by_decorated_name(uname.c_str());
        if (cname == uname.c_str()) m_name = std::move(uname);
        else m_name = cname;
    }

    static inline const T& deref(const void* ptr) {
        return *reinterpret_cast<const T*>(ptr);
    }

    static inline T& deref(void* ptr) {
        return *reinterpret_cast<T*>(ptr);
    }

    // can be overridden in subclasses to compare POD types
    // by comparing each individual member
    virtual bool pod_mems_equals(const T&, const T&) const {
        return false;
    }

    std::string m_name;

 private:

    template<class C>
    typename std::enable_if<std::is_empty<C>::value, bool>::type
    eq(const C&, const C&) const {
        return true;
    }

    template<class C>
    typename std::enable_if<
        !std::is_empty<C>::value && detail::is_comparable<C, C>::value,
        bool
    >::type
    eq(const C& lhs, const C& rhs) const {
        return lhs == rhs;
    }

    template<class C>
    typename std::enable_if<
        !std::is_empty<C>::value
            && std::is_pod<C>::value
            && !detail::is_comparable<C, C>::value,
        bool
    >::type
    eq(const C& lhs, const C& rhs) const {
        return pod_mems_equals(lhs, rhs);
    }

};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_ABSTRACT_UNIFORM_TYPE_INFO_HPP
