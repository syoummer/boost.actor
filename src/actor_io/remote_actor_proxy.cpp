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


#include "boost/actor/to_string.hpp"

#include "boost/actor_io/peer.hpp"
#include "boost/actor_io/middleman.hpp"
#include "boost/actor_io/remote_actor_proxy.hpp"

#include "boost/actor/detail/memory.hpp"
#include "boost/actor/detail/logging.hpp"
#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/sync_request_bouncer.hpp"

using namespace std;

namespace boost {
namespace actor_io {

using namespace actor;

inline sync_request_info* new_req_info(actor_addr sptr, message_id id) {
    return boost::actor::detail::memory::create<sync_request_info>(std::move(sptr), id);
}

sync_request_info::~sync_request_info() { }

sync_request_info::sync_request_info(actor_addr sptr, message_id id)
        : next(nullptr), sender(std::move(sptr)), mid(id) {
}

remote_actor_proxy::remote_actor_proxy(actor_id mid,
                                       node_id_ptr pinfo,
                                       middleman* parent)
        : super(mid), m_parent(parent) {
    BOOST_ACTOR_REQUIRE(parent != nullptr);
    BOOST_ACTOR_LOG_INFO(BOOST_ACTOR_ARG(mid) << ", " << BOOST_ACTOR_TARG(*pinfo, to_string));
    m_node = std::move(pinfo);
}

remote_actor_proxy::~remote_actor_proxy() {
    auto aid = m_id;
    auto node = m_node;
    auto mm = m_parent;
    BOOST_ACTOR_LOG_INFO(BOOST_ACTOR_ARG(m_id) << ", " << BOOST_ACTOR_TSARG(*m_node)
                   << ", protocol = " << detail::demangle(typeid(*m_parent)));
    mm->run_later([aid, node, mm] {
        BOOST_ACTOR_LOGC_TRACE("cppa::io::remote_actor_proxy",
                        "~remote_actor_proxy$run_later",
                        "node = " << to_string(*node) << ", aid " << aid);
        mm->get_namespace().erase(*node, aid);
        auto p = mm->get_peer(*node);
        if (p && p->stop_on_last_proxy_exited()) {
            if (mm->get_namespace().count_proxies(*node) == 0) {
                mm->last_proxy_exited(p);
            }
        }
    });
}

void remote_actor_proxy::deliver(msg_hdr_cref hdr, message msg) {
    // this member function is exclusively called from default_peer from inside
    // the middleman's thread, therefore we can safely access
    // m_pending_requests here
    if (hdr.id.is_response()) {
        // remove this request from list of pending requests
        auto req = hdr.id.request_id();
        m_pending_requests.remove_if([&](const sync_request_info& e) -> bool {
            return e.mid == req;
        });
    }
    hdr.deliver(std::move(msg));
}

void remote_actor_proxy::forward_msg(msg_hdr_cref hdr, message msg) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(m_id) << ", " << BOOST_ACTOR_TSARG(hdr)
                   << ", " << BOOST_ACTOR_TSARG(msg));
    if (hdr.receiver != this) {
        auto cpy = hdr;
        cpy.receiver = this;
        forward_msg(cpy, std::move(msg));
        return;
    }
    if (hdr.sender && hdr.id.is_request()) {
        switch (m_pending_requests.enqueue(new_req_info(hdr.sender, hdr.id))) {
            case boost::actor::detail::enqueue_result::queue_closed: {
                auto rsn = exit_reason();
                m_parent->run_later([rsn, hdr] {
                    BOOST_ACTOR_LOGC_TRACE("cppa::io::remote_actor_proxy",
                                    "forward_msg$bouncer",
                                    "bounce message for reason " << rsn);
                    boost::actor::detail::sync_request_bouncer f{rsn};
                    f(hdr.sender, hdr.id);
                });
                return; // no need to forward message
            }
            case boost::actor::detail::enqueue_result::success: {
                BOOST_ACTOR_LOG_DEBUG("enqueued pending request to non-empty queue");
                break;
            }
            case boost::actor::detail::enqueue_result::unblocked_reader: {
                BOOST_ACTOR_LOG_DEBUG("enqueued pending request to empty queue");
                break;
            }
        }
    }
    auto node = m_node;
    auto mm = m_parent;
    m_parent->run_later([hdr, msg, node, mm] {
        BOOST_ACTOR_LOGC_TRACE("cppa::io::remote_actor_proxy",
                        "forward_msg$forwarder",
                        "");
        mm->deliver(*node, hdr, msg);
    });
}

void remote_actor_proxy::enqueue(msg_hdr_cref hdr, message msg,
                                 execution_unit*) {
    BOOST_ACTOR_REQUIRE(m_parent != nullptr);
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_TARG(hdr, to_string)
                   << ", " << BOOST_ACTOR_TARG(msg, to_string));
    if (       msg.size() == 2
            && msg.type_at(0)->equal_to(typeid(atom_value))
            && msg.get_as<atom_value>(0) == atom("KILL_PROXY")
            && msg.type_at(1)->equal_to(typeid(uint32_t))) {
        BOOST_ACTOR_LOG_DEBUG("received KILL_PROXY message");
        intrusive_ptr<remote_actor_proxy> _this{this};
        auto reason = msg.get_as<uint32_t>(1);
        m_parent->run_later([_this, reason] {
            BOOST_ACTOR_LOGC_TRACE("cppa::io::remote_actor_proxy",
                            "enqueue$kill_proxy_helper",
                            "KILL_PROXY " << to_string(_this->address())
                            << " with exit reason " << reason);
            if (_this->m_pending_requests.closed()) {
                BOOST_ACTOR_LOG_INFO("received KILL_PROXY twice");
            }
            else {
                _this->cleanup(reason);
                boost::actor::detail::sync_request_bouncer f{reason};
                _this->m_pending_requests.close([&](const sync_request_info& e) {
                    f(e.sender, e.mid);
                });
            }
        });
    }
    else forward_msg(hdr, move(msg));
}

void remote_actor_proxy::link_to(const actor_addr& other) {
    if (link_to_impl(other)) {
        // causes remote actor to link to (proxy of) other
        // receiving peer will call: this->local_link_to(other)
        forward_msg({address(), this}, make_message(atom("LINK"), other));
    }
}

void remote_actor_proxy::unlink_from(const actor_addr& other) {
    if (unlink_from_impl(other)) {
        // causes remote actor to unlink from (proxy of) other
        forward_msg({address(), this}, make_message(atom("UNLINK"), other));
    }
}

bool remote_actor_proxy::establish_backlink(const actor_addr& other) {
    if (super::establish_backlink(other)) {
        // causes remote actor to unlink from (proxy of) other
        forward_msg({address(), this}, make_message(atom("LINK"), other));
        return true;
    }
    return false;
}

bool remote_actor_proxy::remove_backlink(const actor_addr& other) {
    if (super::remove_backlink(other)) {
        // causes remote actor to unlink from (proxy of) other
        forward_msg({address(), this}, make_message(atom("UNLINK"), other));
        return true;
    }
    return false;
}

void remote_actor_proxy::local_link_to(const actor_addr& other) {
    link_to_impl(other);
}

void remote_actor_proxy::local_unlink_from(const actor_addr& other) {
    unlink_from_impl(other);
}

} // namespace actor_io
} // namespace boost
