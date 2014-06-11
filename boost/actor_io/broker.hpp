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


#ifndef BOOST_ACTOR_BROKER_HPP
#define BOOST_ACTOR_BROKER_HPP

#include <map>
#include <vector>

#include "boost/actor/spawn.hpp"
#include "boost/actor/extend.hpp"
#include "boost/actor/local_actor.hpp"
#include "boost/actor/accept_handle.hpp"
#include "boost/actor/connection_handle.hpp"

#include "boost/actor_io/network.hpp"

#include "boost/actor/mixin/functor_based.hpp"
#include "boost/actor/mixin/behavior_stack_based.hpp"

#include "boost/actor/policy/not_prioritizing.hpp"
#include "boost/actor/policy/sequential_invoke.hpp"

namespace boost {
namespace actor_io {

class broker;

using broker_ptr = intrusive_ptr<broker>;

/**
 * @brief A broker mediates between an actor system
 *        and other components in the network.
 * @extends local_actor
 */
class broker : public actor::extend<actor::local_actor>::
                      with<actor::mixin::behavior_stack_based<actor::behavior>::impl>,
               public actor::spawn_as_is {

    friend class actor::policy::sequential_invoke;

    typedef combined_type super;

 public:

    using connection_handle = actor::connection_handle;

    using accept_handle = actor::accept_handle;

    using buffer_type = std::vector<char>;

    class servant {

        friend class broker;

     public:

        virtual ~servant();

     protected:

        virtual void remove_from_broker() = 0;

        virtual actor::message disconnect_message() = 0;

        servant(broker* ptr);

        void set_broker(broker* ptr);

        void disconnect();

        bool m_disconnected;

        broker* m_broker;

    };

    class scribe : public network::stream_manager, public servant {

        using super = servant;

     public:

        ~scribe();

        scribe(broker* parent);

        /**
         * @brief Implicitly starts the read loop on first call.
         */
        virtual void configure_read(receive_policy::config config) = 0;

        /**
         * @brief Grants access to the output buffer.
         */
        virtual buffer_type& wr_buf() = 0;

        /**
         * @brief Flushes the output buffer, i.e., sends the content of
         *        the buffer via the network.
         */
        virtual void flush() = 0;

        inline connection_handle hdl() const {
            return m_hdl;
        }

        void io_failure(network::operation op, const std::string& msg) override;

     protected:

        virtual buffer_type& rd_buf() = 0;

        inline actor::new_data_msg& read_msg() {
            return m_read_msg.get_as_mutable<actor::new_data_msg>(0);
        }

        inline const actor::new_data_msg& read_msg() const{
            return m_read_msg.get_as<actor::new_data_msg>(0);
        }

        void remove_from_broker() override;

        actor::message disconnect_message() override;

        void consume(const void* data, size_t num_bytes) override;

        connection_handle m_hdl;

        actor::message m_read_msg;

    };

    class doorman : public network::acceptor_manager, public servant {

        using super = servant;

     public:

        ~doorman();

        doorman(broker* parent);

        inline accept_handle hdl() const {
            return m_hdl;
        }

        void io_failure(network::operation op, const std::string& msg) override;

        // needs to be launched explicitly
        virtual void launch() = 0;

     protected:

        void remove_from_broker() override;

        actor::message disconnect_message() override;

        inline actor::new_connection_msg& accept_msg() {
            return m_accept_msg.get_as_mutable<actor::new_connection_msg>(0);
        }

        inline const actor::new_connection_msg& accept_msg() const {
            return m_accept_msg.get_as<actor::new_connection_msg>(0);
        }

        accept_handle m_hdl;

        actor::message m_accept_msg;

    };

    class continuation;

    // ... and some helpers need friendship
    friend class scribe;
    friend class doorman;
    friend class continuation;

    ~broker();

    /**
     * @brief Modifies the receive policy for given connection.
     * @param hdl Identifies the affected connection.
     * @param config Contains the new receive policy.
     */
    void configure_read(connection_handle hdl,
                        receive_policy::config config);

    /**
     * @brief Returns the write buffer for given connection.
     */
    buffer_type& wr_buf(connection_handle hdl);

    void write(connection_handle hdl,
               size_t buf_size,
               const void* buf);

    /**
     * @brief Sends the content of the buffer for given connection.
     */
    void flush(connection_handle hdl);

    /**
     * @brief Returns the number of open connections.
     */
    inline size_t num_connections() const {
        return m_scribes.size();
    }

    std::vector<connection_handle> connections() const;

    /** @cond PRIVATE */

    template<typename F, typename... Ts>
    actor::actor fork(F fun, connection_handle hdl, Ts&&... vs) {
        auto i = m_scribes.find(hdl);
        if (i == m_scribes.end()) {
            BOOST_ACTOR_LOG_ERROR("invalid handle");
            throw std::invalid_argument("invalid handle");
        }
        auto sptr = i->second;
        BOOST_ACTOR_REQUIRE(sptr->hdl() == hdl);
        m_scribes.erase(i);
        return spawn_functor(nullptr,
                             [sptr](broker* forked) {
                                 sptr->set_broker(forked);
                                 forked->m_scribes.emplace(sptr->hdl(), sptr);
                             },
                             fun,
                             std::forward<Ts>(vs)...);
    }

    template<class Socket>
    connection_handle add_connection(Socket sock) {
        BOOST_ACTOR_LOG_TRACE("");
        class impl : public scribe {

            using super = scribe;

         public:

            impl(broker* parent, Socket&& s)
                    : super(parent)
                    , m_launched(false)
                    , m_stream(s.get_io_service()) {
                m_stream.init(std::move(s));
            }

            void configure_read(receive_policy::config config) override {
                BOOST_ACTOR_LOGM_TRACE("boost::actor_io::broker::scribe", "");
                m_stream.configure_read(config);
                if (!m_launched) launch();
            }

            buffer_type& wr_buf() override {
                return m_stream.wr_buf();
            }

            buffer_type& rd_buf() override {
                return m_stream.rd_buf();
            }

            void stop_reading() override {
                BOOST_ACTOR_LOGM_TRACE("boost::actor_io::broker::scribe", "");
                m_stream.stop_reading();
                disconnect();
            }

            void stop() override {
                BOOST_ACTOR_LOGM_TRACE("boost::actor_io::broker::scribe", "");
                m_stream.stop();
                disconnect();
            }

            void flush() override {
                BOOST_ACTOR_LOGM_TRACE("boost::actor_io::broker::scribe", "");
                m_stream.flush(this);
            }

            void launch() {
                BOOST_ACTOR_LOGM_TRACE("boost::actor_io::broker::scribe", "");
                BOOST_ACTOR_REQUIRE(!m_launched);
                m_launched = true;
                m_stream.start(this);
            }

         private:

            bool m_launched;
            network::stream<Socket> m_stream;

        };
        intrusive_ptr<impl> ptr{new impl{this, std::move(sock)}};
        m_scribes.emplace(ptr->hdl(), ptr);
        return ptr->hdl();
    }

    template<class SocketAcceptor>
    accept_handle add_acceptor(SocketAcceptor fd) {
        class impl : public doorman {

            using super = doorman;

         public:

            impl(broker* parent, SocketAcceptor&& s)
            : super(parent)
            , m_acceptor(s.get_io_service()) {
                m_acceptor.init(std::move(s));
            }

            void new_connection() override {
                accept_msg().handle = m_broker->add_connection(std::move(m_acceptor.accepted_socket()));
                m_broker->invoke_message(actor::invalid_actor_addr,
                                         actor::message_id::invalid,
                                         m_accept_msg);
            }

            void stop_reading() override {
                m_acceptor.stop();
                disconnect();
            }

            void stop() override {
                m_acceptor.stop();
                disconnect();
            }

            void launch() override {
                m_acceptor.start(this);
            }

            network::acceptor<SocketAcceptor> m_acceptor;

        };
        intrusive_ptr<impl> ptr{new impl{this, std::move(fd)}};
        m_doormen.emplace(ptr->hdl(), ptr);
        if (initialized()) ptr->launch();
        return ptr->hdl();
    }

    void enqueue(const actor::actor_addr&,
                 actor::message_id,
                 actor::message,
                 actor::execution_unit*) override;

    template<typename F>
    static broker_ptr from(F fun) {
        // transform to STD function here, because GCC is unable
        // to select proper overload otherwise ...
        typedef decltype(fun((broker*) nullptr)) fres;
        std::function<fres(broker*)> stdfun{std::move(fun)};
        return from_impl(std::move(stdfun));
    }

    template<typename F, typename T, typename... Ts>
    static broker_ptr from(F fun, T&& v, Ts&&... vs) {
        return from(std::bind(fun,
                              std::placeholders::_1,
                              std::forward<T>(v),
                              std::forward<Ts>(vs)...));
    }

    /**
     * @brief Stops read IO operations on @p handle,
     *        but send any pending data.
     */
    void stop_reading(connection_handle handle);

    /**
     * @brief Stops all read IO operations.
     */
    void stop_reading();

    /**
     * @brief Stops both read and write operations on @p handle.
     */
    void stop(connection_handle handle);

    /**
     * @brief Stops accept operations on @p handle.
     */
    void stop(accept_handle handle);

    class functor_based;

    void launch(bool is_hidden, actor::execution_unit*);

 protected:

    broker();

    void cleanup(uint32_t reason) override;

    typedef intrusive_ptr<scribe> scribe_pointer;

    typedef intrusive_ptr<doorman> doorman_pointer;

    bool initialized() const;

    /** @endcond */

    virtual actor::behavior make_behavior() = 0;

 private:

    template<typename Handle, class T>
    static T& by_id(Handle hdl, std::map<Handle, intrusive_ptr<T>>& elements) {
        auto i = elements.find(hdl);
        if (i == elements.end()) {
            throw std::invalid_argument("invalid handle");
        }
        return *(i->second);
    }

    // throws on error
    inline scribe& by_id(connection_handle hdl) {
        return by_id(hdl, m_scribes);
    }

    // throws on error
    inline doorman& by_id(accept_handle hdl) {
        return by_id(hdl, m_doormen);
    }

    void invoke_message(const actor::actor_addr& sender,
                        actor::message_id mid,
                        actor::message& msg);

    bool invoke_message_from_cache();

    void erase_io(int id);

    void erase_acceptor(int id);

    std::map<accept_handle, doorman_pointer> m_doormen;
    std::map<connection_handle, scribe_pointer> m_scribes;

    actor::policy::not_prioritizing  m_priority_policy;
    actor::policy::sequential_invoke m_invoke_policy;

    bool m_initialized;

    bool m_hidden;

    int64_t m_next_handle_id;

    bool m_running;

};

// functions from ref_counted cannot be found by ADL => provide new overload
inline void intrusive_ptr_add_ref(broker* p) {
    p->ref();
}

// functions from ref_counted cannot be found by ADL => provide new overload
inline void intrusive_ptr_release(broker* p) {
    p->deref();
}

class broker::functor_based : public actor::extend<broker>::
                                     with<actor::mixin::functor_based> {

    using super = combined_type;

 public:

    template<typename... Ts>
    functor_based(Ts&&... vs) : super(std::forward<Ts>(vs)...) { }

    ~functor_based();

    actor::behavior make_behavior() override;

};

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_BROKER_HPP
