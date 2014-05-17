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


#ifndef BOOST_ACTOR_DECORATED_TUPLE_HPP
#define BOOST_ACTOR_DECORATED_TUPLE_HPP

#include <vector>
#include <algorithm>

#include "boost/actor/config.hpp"
#include "boost/actor/ref_counted.hpp"
#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/detail/type_list.hpp"

#include "boost/actor/detail/tuple_vals.hpp"
#include "boost/actor/detail/message_data.hpp"
#include "boost/actor/detail/serialize_tuple.hpp"

namespace boost {
namespace actor {
namespace detail {

class decorated_tuple : public message_data {

    typedef message_data super;

    decorated_tuple& operator=(const decorated_tuple&) = delete;

 public:

    typedef std::vector<size_t> vector_type;

    typedef message_data::ptr pointer;

    typedef const std::type_info* rtti;

    // creates a dynamically typed subtuple from @p d with an offset
    static inline pointer create(pointer d, vector_type v) {
        return pointer{new decorated_tuple(std::move(d), std::move(v))};
    }

    // creates a statically typed subtuple from @p d with an offset
    static inline pointer create(pointer d, rtti ti, vector_type v) {
        return pointer{new decorated_tuple(std::move(d), ti, std::move(v))};
    }

    // creates a dynamically typed subtuple from @p d with an offset
    static inline pointer create(pointer d, size_t offset) {
        return pointer{new decorated_tuple(std::move(d), offset)};
    }

    // creates a statically typed subtuple from @p d with an offset
    static inline pointer create(pointer d, rtti ti, size_t offset) {
        return pointer{new decorated_tuple(std::move(d), ti, offset)};
    }

    void* mutable_at(size_t pos) override;

    size_t size() const override;

    decorated_tuple* copy() const override;

    const void* at(size_t pos) const override;

    const uniform_type_info* type_at(size_t pos) const override;

    const std::string* tuple_type_names() const override;

    rtti type_token() const override;

 private:

    pointer     m_decorated;
    rtti        m_token;
    vector_type m_mapping;

    void init();

    void init(size_t);

    decorated_tuple(pointer, size_t);

    decorated_tuple(pointer, rtti, size_t);

    decorated_tuple(pointer, vector_type&&);

    decorated_tuple(pointer, rtti, vector_type&&);

    decorated_tuple(const decorated_tuple&) = default;

};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DECORATED_TUPLE_HPP
