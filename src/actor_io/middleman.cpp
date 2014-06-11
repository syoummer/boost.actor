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

#include "boost/actor_io/middleman.hpp"
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

const node_id& middleman::node() const {
    BOOST_ACTOR_REQUIRE(m_node != nullptr);
    return *m_node;
}

const node_id_ptr& middleman::node_ptr() const {
    BOOST_ACTOR_REQUIRE(m_node != nullptr);
    return m_node;
}

void middleman::add_broker(broker_ptr bptr) {
    m_brokers.emplace(bptr);
    bptr->attach_functor([=](uint32_t) {
        m_brokers.erase(bptr);
    });
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
    BOOST_ACTOR_LOG_TRACE("");
    m_backend.dispatch([=] {
        BOOST_ACTOR_LOGM_TRACE("destroy$lambda", "");
        delete m_supervisor;
        m_supervisor = nullptr;
        // m_managers will be modified while we are stopping each manager,
        // because each manager will call remove(...)
        std::vector<broker_ptr> brokers;
        for (auto& kvp : m_named_brokers) brokers.push_back(kvp.second);
        for (auto& bro : brokers) bro->stop_reading();
    });
    m_thread.join();
    m_named_brokers.clear();
}

void middleman::dispose() {
    delete this;
}

middleman::middleman() : m_supervisor(nullptr) { }

middleman::~middleman() { }

} // namespace actor_io
} // namespace boost
