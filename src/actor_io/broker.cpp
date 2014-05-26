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
#include "boost/actor_io/buffered_writing.hpp"

#include "boost/actor/detail/make_counted.hpp"
#include "boost/actor/detail/actor_registry.hpp"
#include "boost/actor/detail/sync_request_bouncer.hpp"

using std::cout;
using std::endl;

namespace boost {
namespace actor_io {

using namespace boost::actor;
using namespace boost::actor::detail;

namespace {

constexpr size_t default_max_buffer_size = 66560; // max 65kb per default
constexpr size_t default_chunk_size      =  1024; // allocate 1kb chunks

} // namespace <anonymous>

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
        m_self->invoke_message(m_hdr, std::move(m_data));
    }

 private:

    broker_ptr            m_self;
    message_header m_hdr;
    message        m_data;

};

class broker::servant : public continuable {

    typedef continuable super;

 public:

    ~servant();

    template<typename... Ts>
    servant(broker_ptr parent, Ts&&... args)
    : super{std::forward<Ts>(args)...}, m_disconnected{false}
    , m_broker{std::move(parent)} { }

    void io_failed(event_bitmask mask) override {
        if (mask == event::read) disconnect();
    }

    void dispose() override {
        auto ptr = m_broker;
        ptr->erase_io(read_handle());
    }

    void set_broker(broker_ptr new_broker) {
        if (!m_disconnected) m_broker = std::move(new_broker);
    }

 protected:

    void disconnect() {
        if (!m_disconnected) {
            m_disconnected = true;
            if (m_broker->exit_reason() == exit_reason::not_exited) {
                m_broker->invoke_message({invalid_actor_addr,
                                          invalid_actor},
                                         disconnect_message());
            }
        }
    }

    virtual message disconnect_message() = 0;

    bool m_disconnected;

    broker_ptr m_broker;

};

// avoid weak-vtables warning by providing dtor out-of-line
broker::servant::~servant() { }

class broker::scribe : public extend<broker::servant>::
                              with<buffered_writing> {

    typedef combined_type super;

 public:

    ~scribe();

    scribe(broker_ptr parent, input_stream_ptr in, output_stream_ptr out)
            : super{middleman::instance(), out, std::move(parent),
                    in->read_handle(), out->write_handle()}
            , m_is_continue_reading{false}
            , m_policy{broker::at_least}, m_policy_buffer_size{0}, m_in{in} {
        m_read_msg = make_message(new_data_msg{});
        auto& ndm = read_msg();
        ndm.handle = connection_handle::from_int(in->read_handle());
        ndm.buf.resize(default_chunk_size);
        m_wr_pos = 0;
        m_max_buf_size = default_max_buffer_size;
    }

    void receive_policy(broker::policy_flag policy, size_t buffer_size) {
        BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_ARG(policy) << ", " << BOOST_ACTOR_ARG(buffer_size));
        if (not m_disconnected) {
            m_policy = policy;
            m_policy_buffer_size = buffer_size;
            m_max_buf_size = std::max(default_max_buffer_size, buffer_size);
        }
    }

    inline size_t final_buf_size() const {
        return   m_policy == broker::at_most || m_policy == broker::exactly
               ? m_policy_buffer_size
               : m_max_buf_size;
    }

    // checks whether the buffer reached the user-defined size
    // for the next new_data_msg
    inline bool reached_final_buf_size() const {
        return m_wr_pos == final_buf_size();
    }

    inline size_t max_wr_pos() const {
        return std::min(read_msg().buf.size(), final_buf_size());
    }

    continue_reading_result continue_reading() override {
        BOOST_ACTOR_LOG_TRACE("");
        BOOST_ACTOR_REQUIRE(m_wr_pos < max_wr_pos());
        m_is_continue_reading = true;
        auto sg = make_scope_guard([=] {
            m_is_continue_reading = false;
        });
        for (;;) {
            // stop reading if actor finished execution
            if (m_broker->exit_reason() != exit_reason::not_exited) {
                BOOST_ACTOR_LOG_DEBUG("broker already done; exit reason: "
                               << m_broker->exit_reason());
                return continue_reading_result::closed;
            }
            auto& buf = read_msg().buf;
            auto before = m_wr_pos;
            try {
                bool repeat_reading = false;
                do {
                    auto max_wr = max_wr_pos();
                    auto read = m_in->read_some(buf.data() + m_wr_pos,
                                                max_wr - m_wr_pos);
                    m_wr_pos += read;
                    if (m_wr_pos == max_wr && buf.size() < final_buf_size()) {
                        buf.resize(buf.size() + default_chunk_size);
                        repeat_reading = true;
                    }
                    else repeat_reading = false;
                }
                while (repeat_reading);
            }
            catch (std::ios_base::failure&) {
                disconnect();
                return continue_reading_result::failure;
            }
            BOOST_ACTOR_LOG_DEBUG("received " << (buf.size() - before) << " bytes");
            if  ( before == m_wr_pos
               || (m_policy == broker::exactly && m_wr_pos != m_policy_buffer_size)) {
                return continue_reading_result::continue_later;
            }
            if  ( (   m_policy == broker::at_least
                   && buf.size() >= m_policy_buffer_size)
               || m_policy == broker::exactly
               || m_policy == broker::at_most) {
                BOOST_ACTOR_LOG_DEBUG("invoke io actor");
                // make sure buffer size is in sync with its content
                if (m_wr_pos != buf.size()) {
                    buf.resize(m_wr_pos);
                }
                m_broker->invoke_message({invalid_actor_addr, nullptr},
                                         m_read_msg);
                BOOST_ACTOR_LOG_INFO_IF(!m_read_msg.vals()->unique(), "detached buffer");
                // "reset" buffer
                m_wr_pos = 0;
            }
        }
    }

    inline new_data_msg& read_msg() {
        return m_read_msg.get_as_mutable<new_data_msg>(0);
    }

    inline const new_data_msg& read_msg() const{
        return m_read_msg.get_as<new_data_msg>(0);
    }

    template<typename... Ts>
    static std::unique_ptr<scribe> make(Ts&&... args) {
        return std::unique_ptr<scribe>(new scribe(std::forward<Ts>(args)...));
    }

    connection_handle id() const {
        return connection_handle::from_int(m_in->read_handle());
    }

 protected:

    message disconnect_message() override {
        auto hdl = connection_handle::from_int(m_in->read_handle());
        return make_message(connection_closed_msg{hdl});
    }

 private:

    bool m_is_continue_reading;
    broker::policy_flag m_policy;
    size_t m_policy_buffer_size;
    input_stream_ptr m_in;
    message m_read_msg;
    size_t m_wr_pos; // current write position
    size_t m_max_buf_size;

};

// avoid weak-vtables warning by providing dtor out-of-line
broker::scribe::~scribe() { }

class broker::doorman : public broker::servant {

    typedef servant super;

 public:

    ~doorman();

    doorman(broker_ptr parent, acceptor_uptr ptr)
            : super{std::move(parent), ptr->file_handle()} {
        m_accept_msg = make_message(new_connection_msg{});
        accept_msg().source = accept_handle::from_int(ptr->file_handle());
        m_ptr.swap(ptr);
    }

    continue_reading_result continue_reading() override {
        BOOST_ACTOR_LOG_TRACE("");
        for (;;) {
            optional<stream_ptr_pair> opt{none};
            try { opt = m_ptr->try_accept_connection(); }
            catch (std::exception& e) {
                BOOST_ACTOR_LOG_ERROR(to_verbose_string(e));
                static_cast<void>(e); // keep compiler happy
                return continue_reading_result::failure;
            }
            if (opt) {
                using namespace std;
                auto& p = *opt;
                accept_msg().handle = m_broker->add_connection(std::move(p.first),
                                                               std::move(p.second));
                m_broker->invoke_message({invalid_actor_addr, nullptr},
                                         m_accept_msg);
            }
            else return continue_reading_result::continue_later;
       }
    }

    new_connection_msg& accept_msg() {
        return m_accept_msg.get_as_mutable<new_connection_msg>(0);
    }

    template<typename... Ts>
    static std::unique_ptr<doorman> make(Ts&&... args) {
        return std::unique_ptr<doorman>(new doorman(std::forward<Ts>(args)...));
    }

 protected:

    message disconnect_message() override {
        auto hdl = accept_handle::from_int(m_ptr->file_handle());
        return make_message(acceptor_closed_msg{hdl});
    }

 private:

    acceptor_uptr m_ptr;
    message m_accept_msg;

};

// avoid weak-vtables warning by providing dtor out-of-line
broker::doorman::~doorman() { }

void broker::invoke_message(msg_hdr_cref hdr, message msg) {
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
    m_dummy_node.msg = std::move(msg);
    m_dummy_node.mid = hdr.id;
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
    m_dummy_node.msg.reset();
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

void broker::cleanup(std::uint32_t reason) {
    super::cleanup(reason);
    singletons::get_actor_registry()->dec_running();
}

void broker::write(const connection_handle& hdl, size_t num_bytes, const void* buf) {
    auto i = m_io.find(hdl);
    if (i != m_io.end()) i->second->write(num_bytes, buf);
}

void broker::write(const connection_handle& hdl, const charbuf& buf) {
    write(hdl, buf.size(), buf.data());
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
            auto mm = middleman::instance();
            // launch backend now, because user-defined initialization
            // might call functions like add_connection
            for (auto& kvp : self->m_io) {
                BOOST_ACTOR_LOGF_DEBUG("launch scribe " << kvp.second.get());
                mm->continue_reader(kvp.second.get());
            }
            for (auto& kvp : self->m_accept) {
                BOOST_ACTOR_LOGF_DEBUG("launch doorman " << kvp.second.get());
                mm->continue_reader(kvp.second.get());
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

void broker::erase_io(int id) {
    m_io.erase(connection_handle::from_int(id));
}

void broker::erase_acceptor(int id) {
    m_accept.erase(accept_handle::from_int(id));
}

connection_handle broker::add_connection(input_stream_ptr in, output_stream_ptr out) {
    using namespace std;
    auto id = connection_handle::from_int(in->read_handle());
    auto ires = m_io.insert(make_pair(id, scribe::make(this,
                                                       std::move(in),
                                                       std::move(out))));
    BOOST_ACTOR_REQUIRE(ires.second == true);
    // 'launch' backend only if broker is already initialized
    if (initialized()) {
        BOOST_ACTOR_LOG_DEBUG("launch scribe " << ires.first->second.get());
        auto sptr = ires.first->second.get();
        auto mm = middleman::instance();
        mm->continue_reader(sptr);
    }
    return id;
}

accept_handle broker::add_acceptor(acceptor_uptr ptr) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_MARG(ptr, get));
    using namespace std;
    auto id = accept_handle::from_int(ptr->file_handle());
    auto ires = m_accept.insert(make_pair(id, doorman::make(this, std::move(ptr))));
    BOOST_ACTOR_REQUIRE(ires.second == true);
    // 'launch' backend only if broker is already initialized
    if (initialized()) {
        BOOST_ACTOR_LOG_DEBUG("launch doorman " << ires.first->second.get());
        auto aptr = ires.first->second.get();
        auto mm = middleman::instance();
        mm->continue_reader(aptr);
    }
    return id;
}

boost::actor::actor
broker::fork_impl(std::function<behavior (broker*)> fun, connection_handle hdl) {
    BOOST_ACTOR_LOG_TRACE(BOOST_ACTOR_MARG(hdl, id));
    auto i = m_io.find(hdl);
    if (i == m_io.end()) {
        BOOST_ACTOR_LOG_ERROR("invalid handle");
        throw std::invalid_argument("invalid handle");
    }
    scribe* sptr = i->second.get(); // keep non-owning pointer
    auto result = make_counted<default_broker>(fun);
    result->m_io.insert(make_pair(sptr->id(), std::move(i->second)));
    init_and_launch(result);
    sptr->set_broker(result); // set new broker
    m_io.erase(i);
    return {result};
}

boost::actor::actor
broker::fork_impl(std::function<void (broker*)> fun, connection_handle hdl) {
    return fork_impl(std::function<behavior (broker*)>{[=](broker* self)
                                                       -> behavior {
        fun(self);
        return behavior{};
    }}, hdl);
}

void broker::receive_policy(const connection_handle& hdl,
                            broker::policy_flag policy,
                            size_t buffer_size) {
    auto i = m_io.find(hdl);
    if (i != m_io.end()) i->second->receive_policy(policy, buffer_size);
}

broker::~broker() {
    BOOST_ACTOR_LOG_TRACE("");
}

} // namespace actor_io
} // namespace boost
