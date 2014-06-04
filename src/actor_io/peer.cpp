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


#include <cstring>
#include <cstdint>

#include "boost/actor/on.hpp"
#include "boost/actor/all.hpp"
#include "boost/actor/actor.hpp"
#include "boost/actor/to_string.hpp"
#include "boost/actor/exit_reason.hpp"
#include "boost/actor/actor_proxy.hpp"
#include "boost/actor/message_header.hpp"
#include "boost/actor/binary_serializer.hpp"
#include "boost/actor/binary_deserializer.hpp"

#include "boost/actor/detail/logging.hpp"
#include "boost/actor/detail/demangle.hpp"
#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/safe_equal.hpp"
#include "boost/actor/detail/raw_access.hpp"
#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/actor_registry.hpp"
#include "boost/actor/detail/uniform_type_info_map.hpp"

#include "boost/actor_io/peer.hpp"
#include "boost/actor_io/middleman.hpp"

using namespace std;

namespace boost {
namespace actor_io {

using namespace boost::actor;
using namespace boost::actor::detail;

peer::peer(middleman* parent, node_id_ptr peer_ptr)
: m_parent(parent), m_node(peer_ptr)
, m_stop_on_last_proxy_exited(false)
, m_meta_hdr(uniform_typeid<message_header>())
, m_meta_msg(uniform_typeid<message>())
, m_handshake_data(nullptr) {
    m_handle_control_messages = message_handler{
        on(atom("MONITOR"), arg_match) >> [=](const node_id_ptr& node,
                                              actor_id aid) {
            monitor(node, aid);
        },
        on(atom("KILL_PROXY"), arg_match) >> [=](const node_id_ptr& node,
                                                 actor_id aid,
                                                 uint32_t reason) {
            kill_proxy(node, aid, reason);
        },
        on(atom("LINK"), arg_match) >> [=](const actor_addr& sender,
                                           const actor_addr& addr) {
            link(sender, addr);
        },
        on(atom("UNLINK"), arg_match) >> [=](const actor_addr& sender,
                                             const actor_addr& addr) {
            unlink(sender, addr);
        },
        on(atom("ADD_TYPE"), arg_match) >> [=](uint32_t id,
                                               const std::string& name) {
            auto imap = singletons::get_uniform_type_info_map();
            auto uti = imap->by_uniform_name(name);
            m_incoming_types.emplace(id, uti);
        }
    };
}

void peer::handle_write_failure(const std::string& err) {
    BOOST_ACTOR_LOG_TRACE("node = " << (m_node ? to_string(*m_node) : "nullptr")
                          << " error = " << err);
    static_cast<void>(err);
}

void peer::cleanup() {
    if (m_node) {
        parent()->get_namespace().erase(node());
        parent()->del_peer(this);
        m_node.reset();
    }
    parent()->remove(this);
}

void peer::handle_read_failure(const std::string& err) {
    BOOST_ACTOR_LOG_TRACE("node = " << (m_node ? to_string(*m_node) : "nullptr")
                          << " error = " << err);
    static_cast<void>(err);
    cleanup();
}

void peer::monitor(const node_id_ptr& nptr, actor_id aid) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_MARG(nptr, get)
                          << ", " << BOOST_ACTOR_ARG(aid));
    if (!nptr) {
        BOOST_ACTOR_LOG_ERROR("received MONITOR from invalid peer");
        return;
    }
    auto entry = singletons::get_actor_registry()->get_entry(aid);
    auto& pself = parent()->node_ptr();
    if (*nptr == *pself) {
        BOOST_ACTOR_LOG_ERROR("received 'MONITOR' from pself");
    }
    else if (entry.first == nullptr) {
        if (entry.second == exit_reason::not_exited) {
            BOOST_ACTOR_LOG_ERROR("received MONITOR for unknown "
                           "actor id: " << aid);
        }
        else {
            BOOST_ACTOR_LOG_DEBUG("received MONITOR for an actor "
                           "that already finished "
                           "execution; reply KILL_PROXY");
            // this actor already finished execution;
            // reply with KILL_PROXY message
            // get corresponding peer
            enqueue(make_message(atom("KILL_PROXY"), pself, aid, entry.second));
        }
    }
    else {
        BOOST_ACTOR_LOG_DEBUG("attach functor to " << entry.first.get());
        auto mm = parent();
        entry.first->attach_functor([=](uint32_t reason) {
            mm->run_later([=] {
                BOOST_ACTOR_LOGC_TRACE("boost::actor_io::peer",
                                       "monitor$kill_proxy_helper",
                                       "reason = " << reason);
                m_parent->dispatch(*nptr,
                                  {},
                                  make_message(atom("KILL_PROXY"),
                                               pself,
                                               aid,
                                               reason));
            });
        });
    }
}

void peer::proxy_created(const actor_proxy_ptr&) {
    // nop
}

void peer::proxy_registered(const node_id& nid, actor_id) {
    // intercept any proxy to third nodes
    if (nid != node() && nid != m_parent->node()) {
        // assume remote node to have a path to the third node
        m_parent->announce_route(nid, node());
    }
}

void peer::consume(const void* buf, size_t buf_len) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(buf_len) << ", "
                          << BOOST_ACTOR_TARG(m_state, static_cast<int>));
    auto& ns = parent()->get_namespace();
    ns.set_hook(this);
    boost::actor::binary_deserializer bd{buf, buf_len, &ns, &m_incoming_types};
    switch (m_state) {
        case state::await_server_response_size: {
            auto rsize = bd.read<std::uint32_t>();
            configure_read(receive_policy::exactly(rsize));
            m_state = state::await_server_response;
            break;
        }
        case state::await_server_response: {
            BOOST_ACTOR_REQUIRE(m_handshake_data != nullptr);
            auto remote_aid = bd.read<actor_id>();
            auto peer_pid = bd.read<uint32_t>();
            node_id::host_id_type peer_node_id;
            bd.read_raw(node_id::host_id_size, peer_node_id.data());
            auto remote_ifs_size = bd.read<uint32_t>();
            std::set<std::string> remote_ifs;
            for (uint32_t i = 0; i < remote_ifs_size; ++i) {
                auto str = bd.read<std::string>();
                remote_ifs.emplace(std::move(str));
            }
            auto& ifs = *(m_handshake_data->expected_ifs);
            if (!std::includes(ifs.begin(), ifs.end(),
                               remote_ifs.begin(), remote_ifs.end())) {
                auto tostr = [](const std::set<string>& what) -> string {
                    if (what.empty()) return "actor";
                    string tmp;
                    tmp = "typed_actor<";
                    auto i = what.begin();
                    auto e = what.end();
                    tmp += *i++;
                    while (i != e) tmp += *i++;
                    tmp += ">";
                    return tmp;
                };
                auto iface_str = tostr(remote_ifs);
                auto expected_str = tostr(ifs);
                auto& error_msg = *(m_handshake_data->error_msg);
                if (ifs.empty()) {
                    error_msg = "expected remote actor to be a "
                                "dynamically typed actor but found "
                                "a strongly typed actor of type "
                                + iface_str;
                }
                else if (remote_ifs.empty()) {
                    error_msg = "expected remote actor to be a "
                                "strongly typed actor of type "
                                + expected_str +
                                " but found a dynamically typed actor";
                }
                else {
                    error_msg = "expected remote actor to be a "
                                "strongly typed actor of type "
                                + expected_str +
                                " but found a strongly typed actor of type "
                                + iface_str;
                }
                // abort with error
                m_handshake_data->result->set_value(nullptr);
                stop();
            }
            else {
                m_node = make_counted<node_id>(peer_pid, peer_node_id);
                auto ptr = m_parent->get_peer(*m_node);
                abstract_actor_ptr proxy;
                if (ptr) {
                    BOOST_ACTOR_LOG_INFO("multiple connections to "
                                         << to_string(*m_node)
                                         << " (re-use old one)");
                    proxy = m_parent->get_namespace().get(*m_node, remote_aid);
                    BOOST_ACTOR_LOG_WARNING_IF(!proxy,
                                               "no proxy for published actor "
                                               "found although an open "
                                               "connection exists");
                    // discard this peer; there's already an open connection
                    m_node.reset();
                    stop();
                }
                else {
                    // register this peer at middleman
                    m_parent->register_peer(*m_node, this);
                    // complete handshake to server
                    binary_serializer<buffer_type> bs(&wr_buf());
                    auto& pinf = m_parent->node();
                    bs.write_value(pinf.process_id());
                    bs.write_raw(pinf.host_id().size(), pinf.host_id().data());
                    flush();
                    // prepare to receive messages
                    configure_read(receive_policy::exactly(sizeof(uint32_t)));
                    m_state = state::await_msg_size;
                    proxy = m_parent->get_namespace().get_or_put(m_node,
                                                                 remote_aid);
                }
                m_handshake_data->result->set_value(std::move(proxy));
            }
            m_handshake_data = nullptr;
            break;
        }
        case state::await_client_process_info: {
            auto process_id = bd.read<uint32_t>();
            node_id::host_id_type host_id;
            bd.read_raw(node_id::host_id_size, host_id.data());
            m_node.reset(new node_id(process_id, host_id));
            BOOST_ACTOR_LOG_DEBUG("read process info: " << to_string(*m_node));
            if (parent()->node() == *m_node) {
                BOOST_ACTOR_LOG_WARNING("*** middleman warning: "
                                        "incoming connection from self");
                stop();
            }
            else if (!parent()->register_peer(*m_node, this)) {
                BOOST_ACTOR_LOG_WARNING("multiple incoming connections "
                                        "from the same node");
                stop();
            }
            else {
                // initialization done
                m_state = state::await_msg_size;
                configure_read(receive_policy::exactly(sizeof(uint32_t)));
            }
            break;
        }
        case state::await_msg_size: {
            auto msg_size = bd.read<uint32_t>();
            configure_read(receive_policy::exactly(msg_size));
            m_state = state::await_msg;
            break;
        }
        case state::await_msg: {
            message_header hdr;
            m_meta_hdr->deserialize(&hdr, &bd);
            // forward message without deserializing it if we deal with
            // a remote actor as receiver (i.e. operating as hop)
            if (hdr.receiver) {
                auto ptr = dynamic_cast<abstract_actor*>(&(*hdr.receiver));
                if (ptr && ptr->is_proxy()) {
                    m_parent->dispatch(ptr->node(), buf, buf_len);
                    break; // done
                }
            }
            message msg;
            m_meta_msg->deserialize(&msg, &bd);
            BOOST_ACTOR_LOG_DEBUG("deserialized: " << to_string(hdr)
                                  << " " << to_string(msg));
            if (!hdr.receiver) {
                // only system messages are allowed to
                // have an invalid receiver
                if (!m_handle_control_messages(msg)) {
                    BOOST_ACTOR_LOG_WARNING("received message with invalid "
                                            "receiver that isn't a "
                                            "control message");
                }
            }
            else deliver(hdr, std::move(msg));
            configure_read(receive_policy::exactly(sizeof(uint32_t)));
            m_state = state::await_msg_size;
            break;
        }
    }
    ns.set_hook(nullptr);
}

void peer::kill_proxy(const node_id_ptr& node,
                      actor_id aid,
                      uint32_t reason) {
    BOOST_ACTOR_LOG_TRACE("node = " << (node ? to_string(*node) : "-invalid-")
                          << ", " << BOOST_ACTOR_ARG(aid)
                          << ", " << BOOST_ACTOR_ARG(reason));
    if (!node) {
        BOOST_ACTOR_LOG_ERROR("node = nullptr");
        return;
    }
    auto proxy = parent()->get_namespace().get(*node, aid);
    if (proxy) {
        BOOST_ACTOR_LOG_DEBUG("received KILL_PROXY for " << aid
                       << ":" << to_string(*node));
        send_as(proxy, proxy, atom("KILL_PROXY"), reason);
    }
    else {
        BOOST_ACTOR_LOG_INFO("received KILL_PROXY for " << aid
                      << ":" << to_string(*node)
                      << "but didn't found a matching instance "
                      << "in proxy cache");
    }
}

void peer::deliver(msg_hdr_cref hdr, message msg) {
    BOOST_ACTOR_LOG_TRACE("");
    if (hdr.sender && hdr.sender.is_remote()) {
        // is_remote() is guaranteed to return true if and only if
        // the actor is derived from actor_proxy, hence we do not
        // need to use a dynamic_cast here
        auto ptr = static_cast<actor_proxy*>(raw_access::get(hdr.sender));
        ptr->deliver(hdr, std::move(msg));
    }
    else hdr.deliver(std::move(msg));
}

void peer::link(const actor_addr& lhs, const actor_addr& rhs) {
    // this message is sent from default_actor_proxy in link_to and
    // establish_backling to cause the original actor (sender) to establish
    // a link to ptr as well
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_TARG(lhs, to_string) << ", "
                   << BOOST_ACTOR_TARG(rhs, to_string));
    BOOST_ACTOR_LOG_ERROR_IF(!lhs, "received 'LINK' from invalid sender");
    BOOST_ACTOR_LOG_ERROR_IF(!rhs, "received 'LINK' with invalid receiver");
    if (!lhs || !rhs) return;
    auto locally_link_proxy = [](const actor_addr& proxy, const actor_addr& addr) {
        // again, no need to to use a dynamic_cast here
        auto ptr = static_cast<actor_proxy*>(raw_access::get(proxy));
        ptr->local_link_to(addr);
    };
    switch ((lhs.is_remote() ? 0x10 : 0x00) | (rhs.is_remote() ? 0x01 : 0x00)) {
        case 0x00: // both local
        case 0x11: // both remote
            raw_access::get(lhs)->link_to(rhs);
            break;
        case 0x10: // sender is remote
            locally_link_proxy(lhs, rhs);
            break;
        case 0x01: // receiver is remote
            locally_link_proxy(rhs, lhs);
            break;
        default: BOOST_ACTOR_LOG_ERROR("logic error");
    }
}

void peer::unlink(const actor_addr& lhs, const actor_addr& rhs) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_TARG(lhs, to_string) << ", "
                   << BOOST_ACTOR_TARG(rhs, to_string));
    BOOST_ACTOR_LOG_ERROR_IF(!lhs, "received 'UNLINK' from invalid sender");
    BOOST_ACTOR_LOG_ERROR_IF(!rhs, "received 'UNLINK' with invalid target");
    if (!lhs || !rhs) return;
    auto locally_unlink_proxy = [](const actor_addr& proxy, const actor_addr& addr) {
        // again, no need to to use a dynamic_cast here
        auto ptr = static_cast<actor_proxy*>(raw_access::get(proxy));
        ptr->local_unlink_from(addr);
    };
    switch ((lhs.is_remote() ? 0x10 : 0x00) | (rhs.is_remote() ? 0x01 : 0x00)) {
        case 0x00: // both local
        case 0x11: // both remote
            raw_access::get(lhs)->unlink_from(rhs);
            break;
        case 0x10: // sender is remote
            locally_unlink_proxy(lhs, rhs);
            break;
        case 0x01: // receiver is remote
            locally_unlink_proxy(rhs, lhs);
            break;
        default: BOOST_ACTOR_LOG_ERROR("logic error");
    }
}

void peer::add_type_if_needed(const std::string& tname) {
    if (m_outgoing_types.id_of(tname) == 0) {
        auto id = m_outgoing_types.max_id() + 1;
        auto imap = singletons::get_uniform_type_info_map();
        auto uti = imap->by_uniform_name(tname);
        m_outgoing_types.emplace(id, uti);
        enqueue({invalid_actor_addr, nullptr}, make_message(atom("ADD_TYPE"), id, tname));
    }
}

void peer::serialize_msg(msg_hdr_cref hdr,
                         const message& msg,
                         std::vector<char>& wr_buf) {
    BOOST_ACTOR_LOG_TRACE("");
    auto before = static_cast<uint32_t>(wr_buf.size());
    binary_serializer<buffer_type> bs{&wr_buf,
                                      &(parent()->get_namespace()),
                                      &m_outgoing_types};
    auto tname = msg.tuple_type_names();
    add_type_if_needed((tname) ? *tname
                               : get_tuple_type_names(*msg.vals()));
    uint32_t data_size = 0;
    try {
        bs << data_size;
        m_meta_hdr->serialize(&hdr, &bs);
        m_meta_msg->serialize(&msg, &bs);
    }
    catch (std::exception& e) {
        BOOST_ACTOR_LOG_ERROR(to_verbose_string(e));
        cerr << "*** exception in peer::enqueue; "
             << to_verbose_string(e)
             << endl;
        return;
    }
    BOOST_ACTOR_LOG_DEBUG("serialized: " << to_string(hdr)
                          << " " << to_string(msg));
    data_size =   static_cast<uint32_t>((wr_buf.size() - before))
                - static_cast<uint32_t>(sizeof(uint32_t));
    // update size in buffer
    memcpy(wr_buf.data() + before, &data_size, sizeof(uint32_t));
}

network::multiplexer& peer::backend() {
    return m_parent->backend();
}

void peer::register_at_parent() {
    parent()->add(this);
}

void peer::init_handshake_as_client(client_handshake_data* ptr) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(this));
    configure_read(receive_policy::exactly(sizeof(uint32_t)));
    m_state = state::await_server_response_size;
    m_handshake_data = ptr;
}

void peer::init_handshake_as_sever(const actor_addr& addr) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(this));
    auto& pself = m_parent->node();
    auto& buf = wr_buf();
    binary_serializer<std::vector<char>> bs(&buf);
    bs << uint32_t{0}; // size of the message (updated later on)
    bs << addr.id() << pself.process_id();
    bs.write_raw(pself.host_id().size(), pself.host_id().data());
    auto sigs = addr.interface();
    bs << static_cast<uint32_t>(sigs.size());
    for (auto& sig : sigs) bs << sig;
    // update size in buffer
    auto real_size = static_cast<uint32_t>(buf.size() - sizeof(uint32_t));
    memcpy(buf.data(), &real_size, sizeof(uint32_t));
    flush();
    configure_read(receive_policy::exactly(  sizeof(uint32_t)
                                           + node_id::host_id_size));
    m_state = state::await_client_process_info;
}

} // namespace actor_io
} // namespace boost

