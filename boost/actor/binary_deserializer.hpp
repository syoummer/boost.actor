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


#ifndef BOOST_ACTOR_BINARY_DESERIALIZER_HPP
#define BOOST_ACTOR_BINARY_DESERIALIZER_HPP

#include "boost/actor/deserializer.hpp"

namespace boost {
namespace actor {

/**
 * @brief Implements the deserializer interface with
 *        a binary serialization protocol.
 */
class binary_deserializer : public deserializer {

    typedef deserializer super;

 public:

    binary_deserializer(const void* buf, size_t buf_size,
                        actor_namespace* ns = nullptr,
                        type_lookup_table* table = nullptr);

    binary_deserializer(const void* begin, const void* m_end,
                        actor_namespace* ns = nullptr,
                        type_lookup_table* table = nullptr);

    const uniform_type_info* begin_object() override;
    void end_object() override;
    size_t begin_sequence() override;
    void end_sequence() override;
    void read_value(primitive_variant& storage) override;
    void read_raw(size_t num_bytes, void* storage) override;

 private:

    const void* m_pos;
    const void* m_end;

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_BINARY_DESERIALIZER_HPP
