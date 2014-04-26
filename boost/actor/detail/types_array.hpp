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


#ifndef BOOST_ACTOR_TYPES_ARRAY_HPP
#define BOOST_ACTOR_TYPES_ARRAY_HPP

#include <atomic>
#include <typeinfo>

#include "boost/actor/detail/type_list.hpp"
#include "boost/actor/detail/type_traits.hpp"

// forward declarations
namespace boost {
namespace actor {
class uniform_type_info;
const uniform_type_info* uniform_typeid(const std::type_info&);
} // namespace actor
} // namespace boost

namespace boost {
namespace actor {
namespace detail {

enum type_info_impl { std_tinf, cppa_tinf };

// some metaprogramming utility
template<type_info_impl What, bool IsBuiltin, typename T>
struct ta_util;

template<bool IsBuiltin, typename T>
struct ta_util<std_tinf, IsBuiltin, T> {
    static inline const std::type_info* get() { return &(typeid(T)); }
};

template<>
struct ta_util<std_tinf, true, anything> {
    static inline const std::type_info* get() { return nullptr; }
};

template<typename T>
struct ta_util<cppa_tinf, true, T> {
    static inline const uniform_type_info* get() {
        return uniform_typeid(typeid(T));
    }
};

template<>
struct ta_util<cppa_tinf, true, anything> {
    static inline const uniform_type_info* get() { return nullptr; }
};

template<typename T>
struct ta_util<cppa_tinf, false, T> {
    static inline const uniform_type_info* get() { return nullptr; }
};

// only built-in types are guaranteed to be available at static initialization
// time, other types are announced at runtime

// implements types_array
template<bool BuiltinOnlyTypes, typename... T>
struct types_array_impl {
    static constexpr bool builtin_only = true;
    inline bool is_pure() const { return true; }
    // all types are builtin, perform lookup on constuction
    const uniform_type_info* data[sizeof...(T)];
    types_array_impl() : data{ta_util<cppa_tinf, true, T>::get()...} { }
    inline const uniform_type_info* operator[](size_t p) const {
        return data[p];
    }
    typedef const uniform_type_info* const* const_iterator;
    inline const_iterator begin() const { return std::begin(data); }
    inline const_iterator end() const { return std::end(data); }
};

template<typename... T>
struct types_array_impl<false, T...> {
    static constexpr bool builtin_only = false;
    inline bool is_pure() const { return false; }
    // contains std::type_info for all non-builtin types
    const std::type_info* tinfo_data[sizeof...(T)];
    // contains uniform_type_infos for builtin types and lazy initializes
    // non-builtin types at runtime
    mutable std::atomic<const uniform_type_info*> data[sizeof...(T)];
    mutable std::atomic<const uniform_type_info* *> pairs;
    // pairs[sizeof...(T)];
    types_array_impl()
        : tinfo_data{ta_util<std_tinf, detail::is_builtin<T>::value, T>::get()...} {
        bool static_init[sizeof...(T)] = {    !std::is_same<T, anything>::value
                                           && detail::is_builtin<T>::value ...  };
        for (size_t i = 0; i < sizeof...(T); ++i) {
            if (static_init[i]) {
                data[i].store(uniform_typeid(*(tinfo_data[i])),
                              std::memory_order_relaxed);
            }
        }
    }
    inline const uniform_type_info* operator[](size_t p) const {
        auto result = data[p].load();
        if (result == nullptr) {
            auto tinfo = tinfo_data[p];
            if (tinfo != nullptr) {
                result = uniform_typeid(*tinfo);
                data[p].store(result, std::memory_order_relaxed);
            }
        }
        return result;
    }
    typedef const uniform_type_info* const* const_iterator;
    inline const_iterator begin() const {
        auto result = pairs.load();
        if (result == nullptr) {
            auto parr = new const uniform_type_info*[sizeof...(T)];
            for (size_t i = 0; i < sizeof...(T); ++i) {
                parr[i] = (*this)[i];
            }
            if (!pairs.compare_exchange_weak(result, parr, std::memory_order_relaxed)) {
                delete[] parr;
            }
            else {
                result = parr;
            }
        }
        return result;
    }
    inline const_iterator end() const {
        return begin() + sizeof...(T);
    }
};

// a container for uniform_type_information singletons with optimization
// for builtin types; can act as pattern
template<typename... T>
struct types_array : types_array_impl<detail::tl_forall<detail::type_list<T...>,
                                                      detail::is_builtin>::value,
                                      T...> {
    static constexpr size_t size = sizeof...(T);
    typedef detail::type_list<T...> types;
    typedef typename detail::tl_filter_not<types, detail::is_anything>::type
            filtered_types;
    static constexpr size_t filtered_size = detail::tl_size<filtered_types>::value;
    inline bool has_values() const { return false; }
};

// utility for singleton-like access to a types_array
template<typename... T>
struct static_types_array {
    static types_array<T...> arr;
};

template<typename... T>
types_array<T...> static_types_array<T...>::arr;

template<typename TypeList>
struct static_types_array_from_type_list;

template<typename... T>
struct static_types_array_from_type_list<detail::type_list<T...>> {
    typedef static_types_array<T...> type;
};

template<typename... T>
struct static_type_list;

template<typename T>
struct static_type_list<T> {
    static const std::type_info* list;
    static inline const std::type_info* by_offset(size_t offset) {
        return offset == 0 ? list : &typeid(detail::type_list<>);
    }
};

template<typename T>
const std::type_info* static_type_list<T>::list = &typeid(detail::type_list<T>);


// utility for singleton-like access to a type_info instance of a type_list
template<typename T0, typename T1, typename... Ts>
struct static_type_list<T0, T1, Ts...> {
    static const std::type_info* list;
    static inline const std::type_info* by_offset(size_t offset) {
        return offset == 0 ? list : static_type_list<T1, Ts...>::list;
    }
};

template<typename T0, typename T1, typename... Ts>
const std::type_info* static_type_list<T0, T1, Ts...>::list = &typeid(detail::type_list<T0, T1, Ts...>);

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_TYPES_ARRAY_HPP
