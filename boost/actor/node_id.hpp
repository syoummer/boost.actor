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


#ifndef BOOST_ACTOR_PROCESS_INFORMATION_HPP
#define BOOST_ACTOR_PROCESS_INFORMATION_HPP

#include <array>
#include <string>
#include <cstdint>

#include "boost/intrusive_ptr.hpp"

#include "boost/actor/ref_counted.hpp"

#include "boost/actor/detail/comparable.hpp"

namespace boost {
namespace actor {

class serializer;

/**
 * @brief Identifies a process.
 */
class node_id : public ref_counted, detail::comparable<node_id> {

    typedef ref_counted super;

 public:

    ~node_id();

    /**
     * @brief @c libcppa uses 160 bit hashes (20 bytes).
     */
    static constexpr size_t host_id_size = 20;

    /**
     * @brief Represents a 160 bit hash.
     */
    typedef std::array<std::uint8_t, host_id_size> host_id_type;

    /**
     * @brief Copy constructor.
     */
    node_id(const node_id&);

    /**
     * @brief Creates @c this from @p process_id and @p hash.
     * @param process_id System-wide unique process identifier.
     * @param hash Unique node id as hexadecimal string representation.
     */
    node_id(std::uint32_t process_id, const std::string& hash);

    /**
     * @brief Creates @c this from @p process_id and @p hash.
     * @param process_id System-wide unique process identifier.
     * @param node_id Unique node id.
     */
    node_id(std::uint32_t process_id, const host_id_type& node_id);

    /**
     * @brief Identifies the running process.
     * @returns A system-wide unique process identifier.
     */
    inline std::uint32_t process_id() const { return m_process_id; }

    /**
     * @brief Identifies the host system.
     * @returns A hash build from the MAC address of the first network device
     *          and the UUID of the root partition (mounted in "/" or "C:").
     */
    inline const host_id_type& host_id() const { return m_host_id; }

    /** @cond PRIVATE */

    // "inherited" from comparable<node_id>
    int compare(const node_id& other) const;

    static void serialize_invalid(serializer*);

    // for singleton API

    inline void destroy() {
        deref();
    }

    inline void dispose() {
        deref();
    }

    static node_id* create_singleton();

    inline void initialize() { }

    /** @endcond */

 private:

    std::uint32_t m_process_id;
    host_id_type m_host_id;

};

void host_id_from_string(const std::string& hash,
                         node_id::host_id_type& node_id);

bool equal(const std::string& hash,
           const node_id::host_id_type& node_id);

inline bool equal(const node_id::host_id_type& node_id,
                  const std::string& hash) {
    return equal(hash, node_id);
}

/**
 * @brief A smart pointer type that manages instances of
 *        {@link node_id}.
 * @relates node_id
 */
typedef intrusive_ptr<node_id> node_id_ptr;

/**
 * @relates node_id
 */
std::string to_string(const node_id& what);

/**
 * @relates node_id
 */
std::string to_string(const node_id_ptr& what);

/**
 * @brief Converts a {@link node_id::host_id_type node_id}
 *        to a hexadecimal string.
 * @param node_id A unique node identifier.
 * @returns A hexadecimal representation of @p node_id.
 * @relates node_id
 */
std::string to_string(const node_id::host_id_type& node_id);

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_PROCESS_INFORMATION_HPP
