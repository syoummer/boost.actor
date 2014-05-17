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
#include "boost/actor/singletons.hpp"
#include "boost/actor/exit_reason.hpp"
#include "boost/actor/binary_serializer.hpp"
#include "boost/actor/binary_deserializer.hpp"

#include "boost/actor/detail/raw_access.hpp"
#include "boost/actor/detail/single_reader_queue.hpp"

#include "boost/actor/io/acceptor.hpp"
#include "boost/actor/io/middleman.hpp"
#include "boost/actor/io/peer_acceptor.hpp"
#include "boost/actor/io/ipv4_acceptor.hpp"
#include "boost/actor/io/ipv4_io_stream.hpp"
#include "boost/actor/io/remote_actor_proxy.hpp"

namespace {

constexpr uint32_t max_iface_size = 100;

constexpr uint32_t max_iface_clause_size = 500;

typedef std::set<std::string> string_set;

} // namespace <anonymous>

namespace boost {
namespace actor {
namespace detail {

using namespace io;

void publish_impl(abstract_actor_ptr ptr, std::unique_ptr<acceptor> aptr) {
    // begin the scenes, we serialze/deserialize as actor
    actor whom{raw_access::unsafe_cast(ptr.get())};
    BOOST_ACTOR_LOGF_TRACE(BOOST_ACTOR_TARG(whom, to_string) << ", "
                           << BOOST_ACTOR_MARG(aptr, get));
    if (!whom) return;
    get_actor_registry()->put(whom->id(), detail::raw_access::get(whom));
    auto mm = get_middleman();
    auto addr = whom.address();
    auto sigs = whom->interface();
    mm->register_acceptor(addr, new peer_acceptor(mm, std::move(aptr),
                                                  addr, std::move(sigs)));
}

abstract_actor_ptr remote_actor_impl(stream_ptr_pair io, string_set expected) {
    BOOST_ACTOR_LOGF_TRACE("io{" << io.first.get() << ", "
                           << io.second.get() << "}");
    auto mm = get_middleman();
    auto pinf = mm->node();
    std::uint32_t process_id = pinf->process_id();
    // throws on error
    io.second->write(&process_id, sizeof(std::uint32_t));
    io.second->write(pinf->host_id().data(), pinf->host_id().size());
    // deserialize: actor id, process id, node id, interface
    actor_id remote_aid;
    std::uint32_t peer_pid;
    node_id::host_id_type peer_node_id;
    std::uint32_t iface_size;
    std::set<std::string> iface;
    auto& in = io.first;
    // -> actor id
    in->read(&remote_aid, sizeof(actor_id));
    // -> process id
    in->read(&peer_pid, sizeof(std::uint32_t));
    // -> node id
    in->read(peer_node_id.data(), peer_node_id.size());
    // -> interface
    in->read(&iface_size, sizeof(std::uint32_t));
    if (iface_size > max_iface_size) {
        throw std::invalid_argument("Remote actor claims to have more than"
                                    +std::to_string(max_iface_size)+
                                    " message types? Someone is trying"
                                    " something nasty!");
    }
    std::vector<char> strbuf;
    for (std::uint32_t i = 0; i < iface_size; ++i) {
        std::uint32_t str_size;
        in->read(&str_size, sizeof(std::uint32_t));
        if (str_size > max_iface_clause_size) {
            throw std::invalid_argument("Remote actor claims to have a"
                                        " reply_to<...>::with<...> clause with"
                                        " more than"
                                        +std::to_string(max_iface_clause_size)+
                                        " characters? Someone is"
                                        " trying something nasty!");
        }
        strbuf.reserve(str_size + 1);
        strbuf.resize(str_size);
        in->read(strbuf.data(), str_size);
        strbuf.push_back('\0');
        iface.insert(std::string{strbuf.data()});
    }
    // deserialization done, check interface
    if (iface != expected) {
        auto tostr = [](const std::set<std::string>& what) -> std::string {
            if (what.empty()) return "actor";
            std::string tmp;
            tmp = "typed_actor<";
            auto i = what.begin();
            auto e = what.end();
            tmp += *i++;
            while (i != e) tmp += *i++;
            tmp += ">";
            return tmp;
        };
        auto iface_str = tostr(iface);
        auto expected_str = tostr(expected);
        if (expected.empty()) {
            throw std::invalid_argument("expected remote actor to be a "
                                        "dynamically typed actor but found "
                                        "a strongly typed actor of type "
                                        + iface_str);
        }
        if (iface.empty()) {
            throw std::invalid_argument("expected remote actor to be a "
                                        "strongly typed actor of type "
                                        + expected_str +
                                        " but found a dynamically typed actor");
        }
        throw std::invalid_argument("expected remote actor to be a "
                                    "strongly typed actor of type "
                                    + expected_str +
                                    " but found a strongly typed actor of type "
                                    + iface_str);
    }
    auto pinfptr = make_counted<node_id>(peer_pid, peer_node_id);
    if (*pinf == *pinfptr) {
        // this is a local actor, not a remote actor
        BOOST_ACTOR_LOGF_WARNING("remote_actor() called to "
                                 "access a local actor");
        auto ptr = get_actor_registry()->get(remote_aid);
        return ptr;
    }
    struct remote_actor_result { remote_actor_result* next; actor value; };
    std::mutex qmtx;
    std::condition_variable qcv;
    detail::single_reader_queue<remote_actor_result> q;
    mm->run_later([mm, io, pinfptr, remote_aid, &q, &qmtx, &qcv] {
        BOOST_ACTOR_LOGC_TRACE("cppa",
                        "remote_actor$create_connection", "");
        auto pp = mm->get_peer(*pinfptr);
        BOOST_ACTOR_LOGF_INFO_IF(pp, "connection already exists "
                                     "(re-use old one)");
        if (!pp) mm->new_peer(io.first, io.second, pinfptr);
        auto res = mm->get_namespace().get_or_put(pinfptr, remote_aid);
        q.synchronized_enqueue(qmtx, qcv, new remote_actor_result{0, res});
    });
    std::unique_ptr<remote_actor_result> result(q.synchronized_pop(qmtx, qcv));
    BOOST_ACTOR_LOGF_DEBUG(BOOST_ACTOR_MARG(result, get));
    return raw_access::get(result->value);
}

} // namespace detail
} // namespace actor
} // namespace boost
