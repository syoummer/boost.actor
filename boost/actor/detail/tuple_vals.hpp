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


#ifndef BOOST_ACTOR_TUPLE_VALS_HPP
#define BOOST_ACTOR_TUPLE_VALS_HPP

#include <tuple>
#include <stdexcept>

#include "boost/actor/detail/type_list.hpp"

#include "boost/actor/detail/types_array.hpp"
#include "boost/actor/detail/message_data.hpp"
#include "boost/actor/detail/serialize_tuple.hpp"

namespace boost {
namespace actor {
namespace detail {

template<size_t Pos, size_t Max, bool InRange = (Pos < Max)>
struct tup_ptr_access {
    template<class T>
    static inline
    typename std::conditional<std::is_const<T>::value, const void*, void*>::type
    get(size_t pos, T& tup) {
        if (pos == Pos) return &std::get<Pos>(tup);
        return tup_ptr_access<Pos+1,Max>::get(pos,tup);
    }
};

template<size_t Pos, size_t Max>
struct tup_ptr_access<Pos, Max, false> {
    template<class T>
    static inline
    typename std::conditional<std::is_const<T>::value, const void*, void*>::type
    get(size_t, T&) {
         // end of recursion
         return nullptr;
    }
};

template<typename... Ts>
class tuple_vals : public message_data {

    static_assert(sizeof...(Ts) > 0,
                  "tuple_vals is not allowed to be empty");

    typedef message_data super;

 public:

    typedef std::tuple<Ts...> data_type;

    typedef types_array<Ts...> element_types;

    tuple_vals(const tuple_vals&) = default;

    template<typename... Us>
    tuple_vals(Us&&... args) : super(false), m_data(std::forward<Us>(args)...) { }

    const void* native_data() const {
        return &m_data;
    }

    void* mutable_native_data() {
        return &m_data;
    }

    inline data_type& data() {
        return m_data;
    }

    inline const data_type& data() const {
        return m_data;
    }

    size_t size() const {
        return sizeof...(Ts);
    }

    tuple_vals* copy() const {
        return new tuple_vals(*this);
    }

    const void* at(size_t pos) const {
        BOOST_ACTOR_REQUIRE(pos < size());
        return tup_ptr_access<0, sizeof...(Ts)>::get(pos, m_data);
    }

    void* mutable_at(size_t pos) {
        BOOST_ACTOR_REQUIRE(pos < size());
        return const_cast<void*>(at(pos));
    }

    const uniform_type_info* type_at(size_t pos) const {
        BOOST_ACTOR_REQUIRE(pos < size());
        return m_types[pos];
    }

    bool equals(const message_data& other) const {
        if (size() != other.size()) return false;
        const tuple_vals* o = dynamic_cast<const tuple_vals*>(&other);
        if (o) {
            return m_data == (o->m_data);
        }
        return message_data::equals(other);
    }

    const std::type_info* type_token() const {
        return detail::static_type_list<Ts...>::list;
    }

    const std::string* tuple_type_names() const override {
        // produced name is equal for all instances
        static std::string result = get_tuple_type_names(*this);
        return &result;
    }

 private:

    data_type m_data;

    static types_array<Ts...> m_types;

};

template<typename... Ts>
types_array<Ts...> tuple_vals<Ts...>::m_types;

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_TUPLE_VALS_HPP
