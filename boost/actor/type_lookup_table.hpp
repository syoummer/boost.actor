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


#ifndef BOOST_ACTOR_TYPE_LOOKUP_TABLE_HPP
#define BOOST_ACTOR_TYPE_LOOKUP_TABLE_HPP

#include <vector>
#include <memory>
#include <utility>

#include "boost/actor/uniform_type_info.hpp"

namespace boost {
namespace actor {

/**
 *
 * Default types are:
 *
 *  1: {atom_value}
 *  2: {atom_value, uint32_t}
 *  3: {atom_value, node_id}
 *  4: {atom_value, node_id, uint32_t}
 *  5: {atom_value, node_id, uint32_t, uint32_t}
 *  6: {atom_value, actor_ptr}
 *  7: {atom_value, uint32_t, string}
 *
 */
class type_lookup_table {

 public:

    typedef const uniform_type_info* pointer;

    type_lookup_table();

    pointer by_id(uint32_t id) const;

    pointer by_name(const std::string& name) const;

    uint32_t id_of(const std::string& name) const;

    uint32_t id_of(pointer uti) const;

    void emplace(uint32_t id, pointer instance);

    uint32_t max_id() const;

 private:

    typedef std::vector<std::pair<uint32_t, pointer>> container;
    typedef container::value_type value_type;
    typedef container::iterator iterator;
    typedef container::const_iterator const_iterator;

    container m_data;

    const_iterator find(uint32_t) const;

    iterator find(uint32_t);

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_TYPE_LOOKUP_TABLE_HPP
