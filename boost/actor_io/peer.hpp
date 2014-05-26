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


#ifndef BOOST_ACTOR_peer_IMPL_HPP
#define BOOST_ACTOR_peer_IMPL_HPP

#include <map>
#include <vector>
#include <cstdint>

#include "boost/actor/extend.hpp"
#include "boost/actor/node_id.hpp"
#include "boost/actor/actor_proxy.hpp"
#include "boost/actor/message_handler.hpp"
#include "boost/actor/type_lookup_table.hpp"

#include "boost/actor_io/input_stream.hpp"
#include "boost/actor_io/output_stream.hpp"
#include "boost/actor_io/buffered_writing.hpp"
#include "boost/actor_io/default_message_queue.hpp"

namespace boost {
namespace actor_io {

class middleman_impl;

class peer : public actor::extend<continuable>::with<buffered_writing> {

    typedef combined_type super;

    friend class middleman_impl;

 public:

    typedef std::vector<char> buffer_type;

    peer(middleman* parent,
         const input_stream_ptr& in,
         const output_stream_ptr& out,
         actor::node_id_ptr peer_ptr = nullptr);

    continue_reading_result continue_reading() override;

    continue_writing_result continue_writing() override;

    void dispose() override;

    void io_failed(event_bitmask mask) override;

    void enqueue(actor::msg_hdr_cref hdr, const actor::message& msg);

    inline bool stop_on_last_proxy_exited() const {
        return m_stop_on_last_proxy_exited;
    }

    inline const actor::node_id& node() const {
        return *m_node;
    }

 private:

    enum read_state {
        // connection just established; waiting for process information
        wait_for_process_info,
        // wait for the size of the next message
        wait_for_msg_size,
        // currently reading a message
        read_message
    };

    input_stream_ptr m_in;
    read_state m_state;
    actor::node_id_ptr m_node;

    const actor::uniform_type_info* m_meta_hdr;
    const actor::uniform_type_info* m_meta_msg;

    buffer_type m_rd_buf;
    size_t m_rd_buf_pos;
    buffer_type m_wr_buf;

    default_message_queue_ptr m_queue;

    inline default_message_queue& queue() {
        BOOST_ACTOR_REQUIRE(m_queue != nullptr);
        return *m_queue;
    }

    inline void set_queue(const default_message_queue_ptr& queue) {
        m_queue = queue;
    }

    // if this peer was created using remote_actor(), then m_doorman will
    // point to the published actor of the remote node
    bool m_stop_on_last_proxy_exited;

    actor::message_handler m_content_handler;

    actor::type_lookup_table m_incoming_types;
    actor::type_lookup_table m_outgoing_types;

    void monitor(const actor::actor_addr& sender,
                 const actor::node_id_ptr& node,
                 actor::actor_id aid);

    void kill_proxy(const actor::actor_addr& sender,
                    const actor::node_id_ptr& node,
                    actor::actor_id aid,
                    std::uint32_t reason);

    void link(const actor::actor_addr& sender, const actor::actor_addr& ptr);

    void unlink(const actor::actor_addr& sender, const actor::actor_addr& ptr);

    void deliver(actor::msg_hdr_cref hdr, actor::message msg);

    inline void enqueue(const actor::message& msg) {
        enqueue({actor::invalid_actor_addr, nullptr}, msg);
    }

    void enqueue_impl(actor::msg_hdr_cref hdr, const actor::message& msg);

    void add_type_if_needed(const std::string& tname);

};

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_peer_IMPL_HPP
