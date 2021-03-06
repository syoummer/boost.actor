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


#include <string>

#include "boost/actor/deserializer.hpp"
#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/io/buffer.hpp"

#include "boost/actor/detail/to_uniform_name.hpp"

namespace boost {
namespace actor {

deserializer::deserializer(actor_namespace* ns, type_lookup_table* ot)
: m_namespace{ns}, m_incoming_types{ot} { }

deserializer::~deserializer() { }

void deserializer::read_raw(size_t num_bytes, io::buffer& storage) {
    storage.acquire(num_bytes);
    read_raw(num_bytes, storage.data());
    storage.inc_size(num_bytes);
}

} // namespace actor
} // namespace boost
