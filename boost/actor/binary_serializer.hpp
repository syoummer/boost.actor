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


#ifndef BOOST_ACTOR_BINARY_SERIALIZER_HPP
#define BOOST_ACTOR_BINARY_SERIALIZER_HPP

#include <utility>
#include <sstream>
#include <iomanip>
#include <type_traits>

#include "boost/actor/serializer.hpp"
#include "boost/actor/type_lookup_table.hpp"
#include "boost/actor/primitive_variant.hpp"

#include "boost/actor/detail/ieee_754.hpp"
#include "boost/actor/detail/type_traits.hpp"

namespace boost {
namespace actor {

/**
 * @brief Implements the serializer interface with
 *        a binary serialization protocol.
 * @tparam Buffer A class providing a compatible interface to std::vector<char>.
 */
template<class Buffer>
class binary_serializer : public serializer {

    typedef serializer super;

 public:

    class binary_writer : public static_visitor<> {

     public:

        binary_writer(Buffer* sink) : m_sink(sink) { }

        template<typename T>
        static inline void write_int(Buffer* sink, const T& value) {
            auto first = reinterpret_cast<const char*>(&value);
            auto last = first + sizeof(T);
            sink->insert(sink->end(), first, last);
        }

        static inline void write_string(Buffer* sink,
                                        const std::string& str) {
            write_int(sink, static_cast<uint32_t>(str.size()));
            sink->insert(sink->end(), str.begin(), str.end());
        }

        template<typename T>
        void operator()(const T& value,
                        typename std::enable_if<std::is_integral<T>::value>::type* = 0) const {
            write_int(m_sink, value);
        }

        template<typename T>
        void operator()(const T& value,
                        typename std::enable_if<std::is_floating_point<T>::value>::type* = 0) const {
            auto tmp = detail::pack754(value);
            write_int(m_sink, tmp);
        }

        // the IEEE-754 conversion does not work for long double
        // => fall back to string serialization (event though it sucks)
        void operator()(const long double& v) const {
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<long double>::digits)
                << v;
            write_string(m_sink, oss.str());
        }

        void operator()(const atom_value& val) const {
            (*this)(static_cast<uint64_t>(val));
        }

        void operator()(const std::string& str) const {
            write_string(m_sink, str);
        }

        void operator()(const std::u16string& str) const {
            // write size as 32 bit unsigned
            write_int(m_sink, static_cast<uint32_t>(str.size()));
            for (char16_t c : str) {
                // force writer to use exactly 16 bit
                write_int(m_sink, static_cast<uint16_t>(c));
            }
        }

        void operator()(const std::u32string& str) const {
            // write size as 32 bit unsigned
            write_int(m_sink, static_cast<uint32_t>(str.size()));
            for (char32_t c : str) {
                // force writer to use exactly 32 bit
                write_int(m_sink, static_cast<uint32_t>(c));
            }
        }

     private:

        Buffer* m_sink;

    };

    /**
     * @brief Creates a binary serializer writing to @p write_buffer.
     * @warning @p write_buffer must be guaranteed to outlive @p this
     */
    binary_serializer(Buffer* write_buffer,
                      actor_namespace* ns = nullptr,
                      type_lookup_table* lookup_table = nullptr)
            : super(ns, lookup_table), m_sink(write_buffer) { }

    void begin_object(const uniform_type_info* uti) override {
        auto ot = outgoing_types();
        uint32_t id = (ot) ? ot->id_of(uti) : 0;
        uint8_t flag = (id == 0) ? 1 : 0;
        binary_writer::write_int(m_sink, flag);
        if (flag == 1) binary_writer::write_string(m_sink, uti->name());
        else binary_writer::write_int(m_sink, id);
    }

    void end_object() override {
        // NOP
    }

    void begin_sequence(size_t list_size) override {
        binary_writer::write_int(m_sink, static_cast<uint32_t>(list_size));
    }

    void end_sequence() override {
        // NOP
    }

    void write_value(const primitive_variant& value) override {
        apply_visitor(binary_writer{m_sink}, value);
    }

    void write_raw(size_t num_bytes, const void* data) override {
        auto first = reinterpret_cast<const char*>(data);
        auto end = first + num_bytes;
        m_sink->insert(m_sink->end(), first, end);
    }

 private:

    Buffer* m_sink;

};

template<class Buffer, typename T,
         class = typename std::enable_if<detail::is_primitive<T>::value>::type>
binary_serializer<Buffer>& operator<<(binary_serializer<Buffer>& bs,
                                      const T& value) {
    bs.write_value(value);
    return bs;
}

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_BINARY_SERIALIZER_HPP
