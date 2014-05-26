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


#ifndef BOOST_ACTOR_BUFFER_HPP
#define BOOST_ACTOR_BUFFER_HPP

#include <ios>       // ios_base::failure
#include <cstddef>   // size_t
#include <cstring>   // memcmp
#include <iterator>  // std::distance
#include <stdexcept> // invalid_argument

#include "boost/actor_io/max_msg_size.hpp"

namespace boost {
namespace actor_io {

class input_stream;

enum buffer_write_policy {
    grow_if_needed,
    do_not_grow
};

/**
 * @brief A buffer implementation with configurable final size
 *        that also supports dynamic growing if needed.
 */
class buffer {

 public:

    typedef void*          iterator;
    typedef const iterator const_iterator;

    /**
     * @brief Creates an empty buffer.
     */
    buffer();

    /**
     * @brief Creates a buffer holding at most @p max_buffer_size bytes
     *        that allocates memory in steps of @p chunk_size bytes.
     * @note The default chunk_size used by other constuctors is 512 bytes.
     * @note The default maximum buffer size is 16MB.
     */
    buffer(size_t chunk_size, size_t max_buffer_size);

    buffer(buffer&& other);

    buffer(const buffer& other);

    buffer& operator=(buffer&& other);

    buffer& operator=(const buffer& other);

    ~buffer();

    /**
     * @brief Makes sure the buffer can at least write @p num_bytes additional
     *        bytes, increasing the final size if needed.
     */
    void acquire(size_t num_bytes);

    /**
     * @brief Erases the first @p num_bytes bytes from the buffer.
     */
    void erase_leading(size_t num_bytes);

    /**
     * @brief Erases the last @p num_bytes bytes from the buffer.
     */
    void erase_trailing(size_t num_bytes);

    /**
     * @brief Clears the buffer's content.
     */
    inline void clear();

    /**
     * @brief Returns the size of the buffer's content in bytes.
     */
    inline size_t size() const;

    /**
     * @brief Returns the configured final size of this buffer. This value
     *        can be changed by using {@link final_size()} and controls how much
     *        bytes are consumed when using {@link append_from()}.
     */
    inline size_t final_size() const;

    /**
     * @brief Sets the buffer's final size to @p new_value.
     * @throws std::invalid_argument if <tt>new_value > maximum_size()</tt>.
     */
    void final_size(size_t new_value);

    /**
     * @brief Returns the difference between {@link final_size()}
     *        and {@link size()}.
     */
    inline size_t remaining() const;

    /**
     * @brief Returns the buffer's content.
     */
    inline const void* data() const;

    /**
     * @brief Returns the buffer's content.
     */
    inline void* data();

    /**
     * @brief Returns the buffer's content offset by @p offset.
     */
    inline void* offset_data(size_t offset);

    /**
     * @brief Returns the buffer's content offset by @p offset.
     */
    inline const void* offset_data(size_t offset) const;

    /**
     * @brief Checks whether <tt>size() == remaining()</tt>.
     */
    inline bool full() const;

    /**
     * @brief Checks whether <tt>size() == 0</tt>.
     */
    inline bool empty() const;

    /**
     * @brief Returns the maximum size of this buffer.
     */
    inline size_t maximum_size() const;

    /**
     * @brief Sets the maximum size of this buffer.
     */
    inline void maximum_size(size_t new_value);

    /**
     * @brief Writes @p num_bytes from @p data to this buffer.
     * @note The configured final size is ignored
     *       if <tt>wp == grow_if_needed</tt>.
     */
    void write(size_t num_bytes, const void* data, buffer_write_policy wp = grow_if_needed);

    /**
     * @brief Writes the content of @p other to this buffer.
     * @note The configured final size is ignored
     *       if <tt>wp == grow_if_needed</tt>.
     */
    void write(const buffer& other, buffer_write_policy wp = grow_if_needed);

    /**
     * @brief Writes the content of @p other to this buffer.
     * @note The configured final size is ignored
     *       if <tt>wp == grow_if_needed</tt>.
     */
    void write(buffer&& other, buffer_write_policy wp = grow_if_needed);

    /**
     * @brief Appends up to @p remaining() bytes from @p istream to the buffer.
     */
    void append_from(input_stream* istream);

    /**
     * @brief Returns the number of bytes used as minimal allocation unit.
     * @note The default chunk size is 512 bytes.
     */
    inline size_t chunk_size() const;

    /**
     * @brief Sets the number of bytes used as minimal allocation unit.
     * @note The default chunk size is 512 bytes.
     */
    inline void chunk_size(size_t new_value);

    /**
     * @brief Returns the number of currently allocated bytes.
     */
    inline size_t allocated() const;

    /**
     * @brief Resizes the buffer.
     * @note New storage will <b>not</b> be initialized.
     */
    void resize(size_t new_size);

    template<typename Iter>
    void insert(const_iterator pos, Iter first, Iter last);

    inline iterator begin();

    inline iterator end();

    inline const_iterator begin() const;

    inline const_iterator end() const;

 private:

    // pointer to the current write position
    inline char* wr_ptr();

    // called whenever bytes are written to m_data
    inline void inc_size(size_t value);

    // called from erase_* functions to adjust buffer size
    inline void dec_size(size_t value);

    // adjusts alloc_size according to m_chunk_size
    inline size_t adjust(size_t alloc_size) const;

    char*  m_data;
    size_t m_written;
    size_t m_allocated;
    size_t m_final_size;
    size_t m_chunk_size;
    size_t m_max_buffer_size;

};

inline bool operator==(const buffer& lhs, const buffer& rhs) {
    if (lhs.size() == rhs.size()) {
        return memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
    }
    return false;
}

/******************************************************************************
 *             inline and template member function implementations            *
 ******************************************************************************/


inline void buffer::clear() {
    m_written = 0;
}

inline size_t buffer::size() const {
    return m_written;
}

inline size_t buffer::final_size() const {
    return m_final_size;
}

inline size_t buffer::remaining() const {
    return m_final_size - m_written;
}

inline const void* buffer::data() const {
    return m_data;
}

inline void* buffer::data() {
    return m_data;
}

inline void* buffer::offset_data(size_t offset) {
    return m_data + offset;
}

inline const void* buffer::offset_data(size_t offset) const {
    return m_data + offset;
}

inline bool buffer::full() const {
    return remaining() == 0;
}

inline bool buffer::empty() const {
    return size() == 0;
}

inline size_t buffer::maximum_size() const {
    return m_max_buffer_size;
}

inline void buffer::maximum_size(size_t new_value) {
    m_max_buffer_size = new_value;
}

inline char* buffer::wr_ptr() {
    return m_data + m_written;
}

inline void buffer::inc_size(size_t value) {
    m_written += value;
}

inline void buffer::dec_size(size_t value) {
    m_written -= value;
}

inline size_t buffer::chunk_size() const {
    return m_chunk_size;
}

inline void buffer::chunk_size(size_t new_value) {
    m_chunk_size = new_value;
}

inline size_t buffer::allocated() const {
    return m_allocated;
}

inline size_t buffer::adjust(size_t alloc_size) const {
    auto remainder = (alloc_size % m_chunk_size);
    return (remainder == 0) ? alloc_size
                            : (alloc_size - remainder) + m_chunk_size;
}

template<typename Iter>
void buffer::insert(const_iterator pos, Iter first, Iter last) {
    auto dist = std::distance(first, last);
    if (dist < 0) {
        throw std::invalid_argument("distance(first, last) < 0");
    }
    else if (dist == 0) return; // nothing to do
    auto num_bytes = static_cast<size_t>(dist);
    acquire(num_bytes);
    if (num_bytes > remaining()) {
        throw std::ios_base::failure("final buffer size exceeded");
    }
    if (pos == end()) {
        std::copy(first, last, wr_ptr());
    }
    else {
        auto ptr = reinterpret_cast<char*>(const_cast<void*>(pos));
        std::copy_backward(ptr, ptr + num_bytes, ptr + num_bytes);
        std::copy(first, last, ptr);
    }
    inc_size(num_bytes);
}

buffer::iterator buffer::begin() {
    return m_data;
}

buffer::iterator buffer::end() {
    return m_data + m_written;
}

buffer::const_iterator buffer::begin() const {
    return m_data;
}

buffer::const_iterator buffer::end() const {
    return m_data + m_written;
}

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_BUFFER_HPP
