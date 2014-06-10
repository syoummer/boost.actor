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


#ifndef BOOST_ACTOR_OBJECT_ARRAY_HPP
#define BOOST_ACTOR_OBJECT_ARRAY_HPP

#include <vector>

#include "boost/actor/message.hpp"
#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/detail/message_data.hpp"

namespace boost {
namespace actor {

class message_builder {

 public:

    message_builder() = default;

    message_builder(const message_builder&) = delete;
    message_builder& operator=(const message_builder&) = delete;

    template<typename T>
    message_builder& append(T what) {
        return append_impl<T>(std::move(what));
    }

    template<typename Iter>
    message_builder& append(Iter first, Iter last) {
        using vtype = typename detail::rm_const_and_ref<decltype(*first)>::type;
        using converted = typename detail::implicit_conversions<vtype>::type;
        auto uti = uniform_typeid<converted>();
        for (; first != last ; ++first) {
            auto uval = uti->create();
            *reinterpret_cast<converted*>(uval->val) = *first;
            append(std::move(uval));
        }
        return *this;

    }

    message_builder& append(uniform_value what);

    message to_message();

 private:

    template<typename T>
    message_builder&
    append_impl(typename detail::implicit_conversions<T>::type what) {
        typedef decltype(what) type;
        auto uti = uniform_typeid<type>();
        auto uval = uti->create();
        *reinterpret_cast<type*>(uval->val) = std::move(what);
        return append(std::move(uval));
    }

    std::vector<uniform_value> m_elements;

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_OBJECT_ARRAY_HPP
