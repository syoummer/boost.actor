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


#include "boost/actor/config.hpp"

#include <ios> // ios_base::failure
#include <list>
#include <memory>
#include <cstring>    // memset
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <condition_variable>

#ifndef BOOST_ACTOR_WINDOWS
#include <netinet/tcp.h>
#endif

#include "boost/actor/all.hpp"
#include "boost/actor/atom.hpp"
#include "boost/actor/to_string.hpp"
#include "boost/actor/exception.hpp"
#include "boost/actor/exit_reason.hpp"
#include "boost/actor/binary_serializer.hpp"
#include "boost/actor/binary_deserializer.hpp"

#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/raw_access.hpp"
#include "boost/actor/detail/single_reader_queue.hpp"

#include "boost/actor_io/middleman.hpp"
#include "boost/actor_io/peer_acceptor.hpp"
#include "boost/actor_io/remote_actor_proxy.hpp"

namespace boost {
namespace actor_io {
namespace detail {

using boost::actor::detail::raw_access;
using boost::actor::detail::singletons;
using boost::actor::detail::make_counted;

using namespace boost::actor;

void publish_impl(abstract_actor_ptr ptr, std::unique_ptr<acceptor> aptr) {
    // begin the scenes, we serialze/deserialize as actor
    boost::actor::actor whom{raw_access::unsafe_cast(ptr.get())};
    BOOST_ACTOR_LOGF_TRACE(BOOST_ACTOR_TARG(whom, to_string) << ", "
                           << BOOST_ACTOR_MARG(aptr, get));
    if (!whom) return;
    singletons::get_actor_registry()->put(whom->id(), raw_access::get(whom));
    auto mm = middleman::instance();
    auto addr = whom.address();
    auto sigs = whom->interface();
    mm->register_acceptor(addr, new peer_acceptor(mm, std::move(aptr),
                                                  addr, std::move(sigs)));
}

abstract_actor_ptr remote_actor_impl(stream_ptr_pair io, string_set expected) {

}

} // namespace detail
} // namespace actor_io
} // namespace boost
