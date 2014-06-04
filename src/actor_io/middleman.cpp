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


#include <tuple>
#include <cerrno>
#include <memory>
#include <cstring>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "boost/actor/on.hpp"
#include "boost/actor/actor.hpp"
#include "boost/actor/config.hpp"
#include "boost/actor/node_id.hpp"
#include "boost/actor/to_string.hpp"
#include "boost/actor/actor_proxy.hpp"
#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/detail/logging.hpp"
#include "boost/actor/detail/ripemd_160.hpp"
#include "boost/actor/detail/get_root_uuid.hpp"
#include "boost/actor/detail/get_mac_addresses.hpp"

#include "boost/actor_io/peer.hpp"
#include "boost/actor_io/middleman.hpp"
#include "boost/actor_io/peer_acceptor.hpp"
#include "boost/actor_io/remote_actor_proxy.hpp"

#include "boost/actor/detail/safe_equal.hpp"
#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/make_counted.hpp"
#include "boost/actor/detail/actor_registry.hpp"

#ifdef BOOST_ACTOR_WINDOWS
#   include <io.h>
#   include <fcntl.h>
#endif

namespace boost {
namespace actor_io {

using namespace actor;

using boost::actor::detail::make_counted;

middleman* middleman::instance() {
    using namespace boost::actor::detail;
    auto mpi = singletons::middleman_plugin_id;
    return static_cast<middleman*>(singletons::get_plugin_singleton(mpi, [] {
        return new middleman;
    }));
}

const node_id_ptr& middleman::node() const {
    BOOST_ACTOR_REQUIRE(m_node != nullptr);
    return m_node;
}

bool middleman::register_peer(const node_id& node, const peer_ptr& ptr) {
    BOOST_ACTOR_LOG_TRACE("node = " << to_string(node) << ", ptr = " << ptr);
    auto res = m_peers.emplace(node, ptr);
    if (!res.second) {
        BOOST_ACTOR_LOG_WARNING("peer " << to_string(node)
                                << " already defined, "
                                << "multiple calls to remote_actor()?");
        return false;
    }
    BOOST_ACTOR_LOG_INFO("peer " << to_string(node) << " added");
    return true;
}

peer_ptr middleman::get_peer(const node_id& node) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_TARG(node, to_string));
    auto i = m_peers.find(node);
    // future work (?): we *could* try to be smart here and try to
    // route all messages to node via other known peers in the network
    // if i->second.impl == nullptr
    if (i != m_peers.end() && i->second != nullptr) {
        BOOST_ACTOR_LOG_DEBUG("result = " << i->second);
        return i->second;
    }
    BOOST_ACTOR_LOG_DEBUG("result = nullptr");
    return nullptr;
}

void middleman::dispatch(const node_id& node, msg_hdr_cref hdr, message msg) {
    auto i = m_peers.find(node);
    if (i != m_peers.end()) {
        auto ptr = i->second;
        ptr->enqueue(hdr, msg);
    }
    else {
        BOOST_ACTOR_LOG_WARNING("message dropped: no route to node: "
                                << to_string(node));
    }
}

void middleman::last_proxy_exited(const peer_ptr& pptr) {
    BOOST_ACTOR_REQUIRE(pptr != nullptr);
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(pptr)
                   << ", pptr->node() = " << to_string(pptr->node()));
    if (pptr->stop_on_last_proxy_exited()) {
        pptr->stop();
    }
}

void middleman::del_peer(const peer_ptr& pptr) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(pptr));
    auto i = m_peers.find(pptr->node());
    if (i != m_peers.end()) {
        BOOST_ACTOR_LOG_DEBUG_IF(i->second != pptr,
                          "node " << to_string(pptr->node())
                          << " does not exist in m_peers");
        if (i->second == pptr) {
            m_peers.erase(i);
        }
    }
}

actor_proxy_ptr middleman::make_proxy(const node_id_ptr& ptr, actor_id aid) {
    auto mm = this;
    auto res = make_counted<remote_actor_proxy>(aid, std::move(ptr), mm);
    res->attach_functor([=](uint32_t) {
        mm->run_later([=] {
            mm->m_namespace.erase(res);
        });
    });
    return res;
}

void middleman::register_proxy(const node_id& nid, actor_id aid) {
    dispatch(nid, {}, make_message(atom("MONITOR"), m_node, aid));
}

void middleman::initialize() {
    BOOST_ACTOR_LOG_TRACE("");
    m_node = boost::actor::detail::singletons::get_node_id();
    m_supervisor = new network::supervisor{m_backend};
    m_thread = std::thread([this] {
        BOOST_ACTOR_LOGF_TRACE("");
        m_backend.run();
    });
}

void middleman::destroy() {
    m_backend.dispatch([=] {
        delete m_supervisor;
        m_supervisor = nullptr;
        // m_managers will be modified while we are stopping each manager,
        // because each manager will call remove(...)
        std::vector<network::manager_ptr> managers{m_managers.begin(),
                                                   m_managers.end()};
        for (auto& mgr : managers) mgr->stop_reading();
    });
    m_thread.join();
    m_managers.clear();
    m_peers.clear();
}

void middleman::dispose() {
    delete this;
}

void middleman::add(const network::manager_ptr& mgr) {
    run_later([=] { m_managers.emplace(mgr); });
}

void middleman::remove(const network::manager_ptr& mgr) {
    run_later([=] { m_managers.erase(mgr); });
}

middleman::middleman() : m_supervisor(nullptr), m_namespace(*this) { }

middleman::~middleman() { }

} // namespace actor_io
} // namespace boost
