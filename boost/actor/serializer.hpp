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


#ifndef BOOST_ACTOR_SERIALIZER_HPP
#define BOOST_ACTOR_SERIALIZER_HPP

#include <string>
#include <cstddef> // size_t

#include "boost/actor/uniform_type_info.hpp"
#include "boost/actor/primitive_variant.hpp"

#include "boost/actor/detail/to_uniform_name.hpp"

namespace boost {
namespace actor {

class actor_namespace;
class uniform_type_info;
class type_lookup_table;

/**
 * @ingroup TypeSystem
 * @brief Technology-independent serialization interface.
 */
class serializer {

    serializer(const serializer&) = delete;
    serializer& operator=(const serializer&) = delete;

 public:

    /**
     * @note @p addressing must be guaranteed to outlive the serializer
     */
    serializer(actor_namespace* addressing = nullptr,
               type_lookup_table* outgoing_types = nullptr);

    virtual ~serializer();

    /**
     * @brief Begins serialization of an object of type @p uti.
     */
    virtual void begin_object(const uniform_type_info* uti) = 0;

    /**
     * @brief Ends serialization of an object.
     */
    virtual void end_object() = 0;

    /**
     * @brief Begins serialization of a sequence of size @p num.
     */
    virtual void begin_sequence(size_t num) = 0;

    /**
     * @brief Ends serialization of a sequence.
     */
    virtual void end_sequence() = 0;

    /**
     * @brief Writes a single value to the data sink.
     * @param value A primitive data value.
     */
    virtual void write_value(const primitive_variant& value) = 0;

    /**
     * @brief Writes a raw block of data.
     * @param num_bytes The size of @p data in bytes.
     * @param data Raw data.
     */
    virtual void write_raw(size_t num_bytes, const void* data) = 0;

    /**
     * @brief Writes @p num values as a tuple to the data sink.
     * @param num Size of the array @p values.
     * @param values An array of size @p num of primitive data values.
     */
    virtual void write_tuple(size_t num, const primitive_variant* values) = 0;

    inline actor_namespace* get_namespace() {
        return m_namespace;
    }

    inline type_lookup_table* outgoing_types() {
        return m_outgoing_types;
    }

 private:

    actor_namespace* m_namespace;
    type_lookup_table* m_outgoing_types;

};

/**
 * @brief Serializes a value to @p s.
 * @param s A valid serializer.
 * @param what A value of an announced or primitive type.
 * @returns @p s
 * @relates serializer
 */
template<typename T>
serializer& operator<<(serializer& s, const T& what) {
    auto mtype = uniform_typeid<T>();
    if (mtype == nullptr) {
        throw std::logic_error(  "no uniform type info found for "
                               + detail::to_uniform_name(typeid(T)));
    }
    mtype->serialize(&what, &s);
    return s;
}

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_SERIALIZER_HPP
