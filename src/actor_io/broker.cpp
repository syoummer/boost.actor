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


#include <iostream>

#include "boost/none.hpp"

#include "boost/actor/config.hpp"

#include "boost/actor/detail/logging.hpp"
#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/scope_guard.hpp"

#include "boost/actor_io/broker.hpp"
#include "boost/actor_io/middleman.hpp"

#include "boost/actor/detail/make_counted.hpp"
#include "boost/actor/detail/actor_registry.hpp"
#include "boost/actor/detail/sync_request_bouncer.hpp"

using std::cout;
using std::endl;

namespace boost {
namespace actor_io {

using namespace boost::actor;
using namespace boost::actor::detail;

broker::servant::~servant() {
    // nop
}

broker::servant::servant(broker* ptr) : m_disconnected(false), m_broker(ptr) { }

void broker::servant::set_broker(broker* new_broker) {
    if (!m_disconnected) m_broker = new_broker;
}

void broker::servant::disconnect() {
    if (!m_disconnected) {
        m_disconnected = true;
        if (m_broker->exit_reason() == exit_reason::not_exited) {
            auto msg = disconnect_message();
            m_broker->invoke_message({}, msg);
        }
    }
}

broker::scribe::scribe(broker* parent, connection_handle hdl) : super(parent) {
    std::vector<char> tmp;
    m_read_msg = make_message(new_data_msg{hdl, std::move(tmp)});
}


broker::scribe::~scribe() {
    // nop
}

message broker::scribe::disconnect_message() {
    return make_message(connection_closed_msg{id()});
}

void broker::scribe::consume(const void*, size_t num_bytes) {
    auto& buf = rd_buf();
    buf.resize(num_bytes);                    // make sure size is correct
    read_msg().buf.swap(buf);                 // swap into message to client
    m_broker->invoke_message({}, m_read_msg); // call client
    read_msg().buf.swap(buf);                 // swap buffer back to stream
}

void broker::scribe::io_failure(network::operation, const std::string&) {
    disconnect();
}

broker::doorman::doorman(broker* parent, accept_handle hdl) : super{parent} {
    m_accept_msg = make_message(new_connection_msg{});
    accept_msg().source = hdl;
}

broker::doorman::~doorman() {
    // nop
}

message broker::doorman::disconnect_message() {
    return make_message(acceptor_closed_msg{id()});
}

void broker::doorman::io_failure(network::operation, const std::string&) {
    disconnect();
}

class default_broker : public broker {

 public:

    typedef std::function<behavior (broker*)> function_type;

    default_broker(function_type f) : m_fun(std::move(f)) { }

    behavior make_behavior() override {
        return m_fun(this);
    }

 private:

    function_type m_fun;

};

class broker::continuation {

 public:

    continuation(broker_ptr ptr, msg_hdr_cref hdr, message&& msg)
    : m_self(std::move(ptr)), m_hdr(hdr), m_data(move(msg)) { }

    inline void operator()() {
        BOOST_ACTOR_PUSH_AID(m_self->id());
        BOOST_ACTOR_LOG_TRACE("");
        m_self->invoke_message(m_hdr, m_data);
    }

 private:

    broker_ptr     m_self;
    message_header m_hdr;
    message        m_data;

};

void broker::invoke_message(msg_hdr_cref hdr, message& msg) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_TARG(msg, to_string));
    if (   planned_exit_reason() != exit_reason::not_exited
        || bhvr_stack().empty()) {
        BOOST_ACTOR_LOG_DEBUG("actor already finished execution"
                       << ", planned_exit_reason = " << planned_exit_reason()
                       << ", bhvr_stack().empty() = " << bhvr_stack().empty());
        if (hdr.id.valid()) {
            sync_request_bouncer srb{exit_reason()};
            srb(hdr.sender, hdr.id);
        }
        return;
    }
    // prepare actor for invocation of message handler
    m_dummy_node.sender = hdr.sender;
    m_dummy_node.mid = hdr.id;
    std::swap(msg, m_dummy_node.msg);
    try {
        auto bhvr = bhvr_stack().back();
        auto mid = bhvr_stack().back_id();
        switch (m_invoke_policy.handle_message(this, &m_dummy_node, bhvr, mid)) {
            case policy::hm_msg_handled: {
                BOOST_ACTOR_LOG_DEBUG("handle_message returned hm_msg_handled");
                while (   !bhvr_stack().empty()
                       && planned_exit_reason() == exit_reason::not_exited
                       && invoke_message_from_cache()) {
                    // rinse and repeat
                }
                break;
            }
            case policy::hm_drop_msg:
                BOOST_ACTOR_LOG_DEBUG("handle_message returned hm_drop_msg");
                break;
            case policy::hm_skip_msg:
            case policy::hm_cache_msg: {
                BOOST_ACTOR_LOG_DEBUG("handle_message returned hm_skip_msg or hm_cache_msg");
                auto e = mailbox_element::create(hdr, std::move(m_dummy_node.msg));
                m_priority_policy.push_to_cache(unique_mailbox_element_pointer{e});
                break;
            }
        }
    }
    catch (std::exception& e) {
        BOOST_ACTOR_LOG_ERROR("broker killed due to an unhandled exception: "
                       << to_verbose_string(e));
        // keep compiler happy in non-debug mode
        static_cast<void>(e);
        quit(exit_reason::unhandled_exception);
    }
    catch (...) {
        BOOST_ACTOR_LOG_ERROR("broker killed due to an unhandled exception");
        quit(exit_reason::unhandled_exception);
    }
    // restore dummy node
    m_dummy_node.sender = invalid_actor_addr;
    std::swap(m_dummy_node.msg, msg);
    // cleanup if needed
    if (planned_exit_reason() != exit_reason::not_exited) {
        cleanup(planned_exit_reason());
        // release implicit reference count held by MM
        deref();
    }
    else if (bhvr_stack().empty()) {
        BOOST_ACTOR_LOG_DEBUG("bhvr_stack().empty(), quit for normal exit reason");
        quit(exit_reason::normal);
        cleanup(planned_exit_reason());
        // release implicit reference count held by MM
        deref();
    }
}

bool broker::invoke_message_from_cache() {
    BOOST_ACTOR_LOG_TRACE("");
    auto bhvr = bhvr_stack().back();
    auto mid = bhvr_stack().back_id();
    auto e = m_priority_policy.cache_end();
    BOOST_ACTOR_LOG_DEBUG(std::distance(m_priority_policy.cache_begin(), e)
                   << " elements in cache");
    for (auto i = m_priority_policy.cache_begin(); i != e; ++i) {
        auto res = m_invoke_policy.invoke_message(this, *i, bhvr, mid);
        if (res || !*i) {
            m_priority_policy.cache_erase(i);
            if (res) return true;
            return invoke_message_from_cache();
        }
    }
    return false;
}

void broker::enqueue(msg_hdr_cref hdr,
                     message msg,
                     execution_unit*) {
    middleman::instance()->run_later(continuation{this, hdr, std::move(msg)});
}

bool broker::initialized() const {
    return m_initialized;
}

broker::broker() : m_initialized(false) {
    // acquire implicit reference count held by the middleman
    ref();
    // actor is running now
    singletons::get_actor_registry()->inc_running();
}

void broker::cleanup(uint32_t reason) {
    super::cleanup(reason);
    singletons::get_actor_registry()->dec_running();
    for (auto& kvp : m_doormen) {
        kvp.second->stop_reading();
    }
    for (auto& kvp : m_scribes) {
        kvp.second->stop_reading();
    }
}

broker_ptr init_and_launch(broker_ptr ptr) {
    BOOST_ACTOR_PUSH_AID(ptr->id());
    BOOST_ACTOR_LOGF_TRACE("init and launch broker with id " << ptr->id());
    // we want to make sure initialization is executed in MM context
    auto self = ptr.get();
    ptr->become(
        on(atom("INITMSG")) >> [self] {
            BOOST_ACTOR_LOGF_TRACE(BOOST_ACTOR_ARG(self));
            self->unbecome();
            // launch backends now, because user-defined initialization
            // might call functions like add_connection
            for (auto& kvp : self->m_doormen) {
                kvp.second->launch();
            }
            for (auto& kvp : self->m_scribes) {
                kvp.second->launch();
            }
            self->m_initialized = true;
            // run user-defined initialization code
            auto bhvr = self->make_behavior();
            if (bhvr) self->become(std::move(bhvr));
        }
    );
    ptr->enqueue({invalid_actor_addr, ptr},
                  make_message(atom("INITMSG")),
                  nullptr);
    return ptr;
}

broker_ptr broker::from_impl(std::function<behavior (broker*)> fun) {
    return make_counted<default_broker>(fun);
}

broker_ptr broker::from_impl(std::function<void (broker*)> fun) {
    return from([=](broker* self) -> behavior {
        fun(self);
        return {};
    });
}

boost::actor::actor
broker::fork_impl(std::function<behavior (broker*)> f, connection_handle hdl) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_MARG(hdl, id));
    auto i = m_scribes.find(hdl);
    if (i == m_scribes.end()) {
        BOOST_ACTOR_LOG_ERROR("invalid handle");
        throw std::invalid_argument("invalid handle");
    }
    auto sptr = i->second;
    auto result = make_counted<default_broker>(f);
    result->m_scribes.emplace(sptr->id(), i->second);
    init_and_launch(result);
    sptr->set_broker(result.get()); // set new broker
    m_scribes.erase(i);
    return {result};
}

boost::actor::actor
broker::fork_impl(std::function<void (broker*)> f, connection_handle hdl) {
    return fork_impl(std::function<behavior (broker*)>{[=](broker* self)
                                                       -> behavior {
        f(self);
        return behavior{};
    }}, hdl);
}

broker::scribe& broker::by_id(const connection_handle& hdl) {
    auto i = m_scribes.find(hdl);
    if (i == m_scribes.end()) {
        throw std::invalid_argument("invalid connection handle");
    }
    return *(i->second);
}

void broker::configure_read(const connection_handle &hdl,
                            receive_policy::config config) {
    by_id(hdl).configure_read(config);
}

void broker::flush(const connection_handle& hdl) {
    by_id(hdl).flush();
}

broker::buffer_type& broker::wr_buf(const connection_handle& hdl) {
    return by_id(hdl).wr_buf();
}

broker::~broker() {
    BOOST_ACTOR_LOG_TRACE("");
}

} // namespace actor_io
} // namespace boost
