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


#ifndef BOOST_ACTOR_BINARY_SERIALIZER_HPP
#define BOOST_ACTOR_BINARY_SERIALIZER_HPP

#include <utility>

#include "boost/actor/serializer.hpp"

namespace boost {
namespace actor {

namespace io { class buffer; }
namespace detail { class binary_writer; }

/**
 * @brief Implements the serializer interface with
 *        a binary serialization protocol.
 */
class binary_serializer : public serializer {

    typedef serializer super;

 public:

    /**
     * @brief Creates a binary serializer writing to @p write_buffer.
     * @warning @p write_buffer must be guaranteed to outlive @p this
     */
    binary_serializer(io::buffer* write_buffer,
                      actor_namespace* ns = nullptr,
                      type_lookup_table* lookup_table = nullptr);

    void begin_object(const uniform_type_info*) override;

    void end_object() override;

    void begin_sequence(size_t list_size) override;

    void end_sequence() override;

    void write_value(const primitive_variant& value) override;

    void write_tuple(size_t size, const primitive_variant* values) override;

    void write_raw(size_t num_bytes, const void* data) override;

 private:

    io::buffer* m_sink;

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_BINARY_SERIALIZER_HPP
