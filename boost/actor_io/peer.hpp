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

#include <set>
#include <map>
#include <mutex>
#include <string>
#include <future>
#include <vector>
#include <cstdint>
#include <functional>

#include "boost/asio.hpp"

#include "boost/system/error_code.hpp"

#include "boost/actor/node_id.hpp"
#include "boost/actor/actor_proxy.hpp"
#include "boost/actor/ref_counted.hpp"
#include "boost/actor/actor_namespace.hpp"
#include "boost/actor/mailbox_element.hpp"
#include "boost/actor/message_handler.hpp"
#include "boost/actor/type_lookup_table.hpp"

#include "boost/actor/detail/memory.hpp"
#include "boost/actor/detail/single_reader_queue.hpp"

#include "boost/actor_io/network.hpp"

namespace boost {
namespace actor_io {

class middleman;
class peer_acceptor;

class peer : public network::stream_manager
           , public actor::actor_namespace::hook {

    friend class middleman;
    friend class peer_acceptor;

 public:

    typedef std::vector<char> buffer_type;

    typedef actor::detail::single_reader_queue<
                actor::mailbox_element,
                actor::detail::disposer
            >
            mailbox_type;

    virtual void enqueue(actor::msg_hdr_cref hdr, actor::message msg) = 0;

    /**
     * @brief Sends @p data to the remote node.
     * @warning Must not be called outside of the event loop of the middleman.
     */
    virtual void send(const void* data, size_t data_len) = 0;

    inline bool stop_on_last_proxy_exited() const {
        return m_stop_on_last_proxy_exited;
    }

    inline const actor::node_id& node() const {
        return *m_node;
    }

    void consume(const void* data, size_t num_bytes) override;

    /**
     * @brief Creates and starts a new peer connection.
     */
    template<class Stream>
    class impl;

    template<class Socket>
    static intrusive_ptr<impl<network::stream<Socket>>> make(middleman* parent,
                                                             Socket fd);

    void register_at_parent();

    struct client_handshake_data {
        std::promise<actor::abstract_actor_ptr>* result;
        std::string* error_msg;
        const std::set<std::string>* expected_ifs;
    };

    void proxy_created(const actor::actor_proxy_ptr& ptr) override;

    void proxy_registered(const actor::node_id&, actor::actor_id) override;


 protected:

    virtual void stop() = 0;

    virtual void configure_read(receive_policy::config config) = 0;

    // get write buffer
    virtual buffer_type& wr_buf() = 0;

    // flush write buffer
    virtual void flush() = 0;

    inline middleman* parent() {
        return m_parent;
    }

    network::multiplexer& backend();

    void enqueue(actor::message msg) {
        enqueue({}, std::move(msg));
    }

    void cleanup();

    void handle_write_failure(const std::string& ec);

    void handle_read_failure(const std::string& ec);

    peer(middleman* parent, actor::node_id_ptr peer_ptr = nullptr);

    void monitor(const actor::node_id_ptr& node, actor::actor_id aid);

    void kill_proxy(const actor::node_id_ptr& node,
                    actor::actor_id aid,
                    uint32_t reason);

    void link(const actor::actor_addr& sender, const actor::actor_addr& ptr);

    void unlink(const actor::actor_addr& sender, const actor::actor_addr& ptr);

    void deliver(actor::msg_hdr_cref hdr, actor::message msg);

    void serialize_msg(actor::msg_hdr_cref hdr,
                       const actor::message& msg,
                       std::vector<char>& wr_buf);

    void add_type_if_needed(const std::string& tname);

    // handshake handling

    void init_handshake_as_client(client_handshake_data* ptr);

    void init_handshake_as_sever(const actor::actor_addr& published_actor);

    // member variables

    enum class state {
        //
        await_server_response_size,
        //
        await_server_response,
        //
        await_client_process_info,
        // wait for the size of the next message
        await_msg_size,
        // currently reading a message
        await_msg
    };

    middleman*                       m_parent;
    actor::node_id_ptr               m_node;
    state                            m_state;
    bool                             m_stop_on_last_proxy_exited;
    const actor::uniform_type_info*  m_meta_hdr;
    const actor::uniform_type_info*  m_meta_msg;
    actor::type_lookup_table         m_incoming_types;
    actor::type_lookup_table         m_outgoing_types;
    actor::message_handler           m_handle_control_messages;
    client_handshake_data*           m_handshake_data;

};

/**
 * @brief Convenience typedef for peer smart pointers.
 * @relates peer
 */
using peer_ptr = intrusive_ptr<peer>;

template<class Stream>
class peer::impl : public peer {

    typedef peer super;

 public:

    impl(middleman* mm) : super(mm), m_stream(backend()) { }

    inline Stream& get_stream() {
        return m_stream;
    }

    void stop_reading() override {
        m_stream.stop_reading();
        cleanup();
    }

    actor::abstract_actor_ptr start_client(const std::set<std::string>& ifs) {
        std::promise<actor::abstract_actor_ptr> result_promise;
        std::string err;
        client_handshake_data data{&result_promise, &err, &ifs};
        init_handshake_as_client(&data);
        m_stream.start(this);
        auto result = result_promise.get_future().get();
        if (!err.empty()) throw std::runtime_error(err);
        return result;
    }

    void start_server(const actor::actor_addr& published_actor) {
        init_handshake_as_sever(published_actor);
        m_stream.start(this);
    }

    void send(const void* data, size_t data_len) {
        auto first = reinterpret_cast<const char*>(data);
        auto last = first + data_len;
        auto& buf = m_stream.wr_buf();
        buf.insert(buf.end(), first, last);
        m_stream.flush(this);
    }

    void enqueue(actor::msg_hdr_cref hdr, actor::message msg) override {
        m_stream.backend().dispatch([=] {
            BOOST_ACTOR_LOGM_TRACE("boost.actor_io.peer$enqueue_lambda", "");
            serialize_msg(hdr, msg, m_stream.wr_buf());
            m_stream.flush(this);
        });
    }

 protected:

    void stop() override {
        m_stream.stop();
        cleanup();
    }

    buffer_type& wr_buf() override {
        return get_stream().wr_buf();
    }

    void flush() override {
        get_stream().flush(this);
    }

    void configure_read(receive_policy::config config) override {
        m_stream.configure_read(config);
    }

    void io_failure(network::operation op, const std::string& err) override {
        switch (op) {
            case network::operation::read:
                handle_read_failure(err);
                break;
            case network::operation::write:
                handle_write_failure(err);
                break;
        }
    }

 private:

    Stream m_stream;

};

template<class Socket>
intrusive_ptr<peer::impl<network::stream<Socket>>> peer::make(middleman* mm,
                                                              Socket fd) {
    auto result = new impl<network::stream<Socket>>{mm};
    result->register_at_parent();
    result->get_stream().init(std::move(fd));
    return result;
}

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_peer_IMPL_HPP
