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

#include "boost/actor/extend.hpp"
#include "boost/actor/local_actor.hpp"
#include "boost/actor/accept_handle.hpp"
#include "boost/actor/connection_handle.hpp"

#include "boost/actor_io/network.hpp"
#include "boost/actor_io/middleman.hpp"

#include "boost/actor/mixin/behavior_stack_based.hpp"

#include "boost/actor/policy/not_prioritizing.hpp"
#include "boost/actor/policy/sequential_invoke.hpp"

namespace boost {
namespace actor_io {

class broker;

typedef intrusive_ptr<broker> broker_ptr;

broker_ptr init_and_launch(broker_ptr);

/**
 * @brief A broker mediates between an actor system
 *        and other components in the network.
 * @extends local_actor
 */
class broker : public actor::extend<actor::local_actor>::
                      with<actor::mixin::behavior_stack_based<actor::behavior>::impl> {

    friend class actor::policy::sequential_invoke;

    typedef combined_type super;

 public:

    typedef std::vector<char> buffer_type;

    class servant {

        friend class broker;

     public:

        virtual ~servant();

        virtual void launch() = 0;

     protected:

        virtual actor::message disconnect_message() = 0;

        servant(broker* ptr);

        void set_broker(broker* ptr);

        void disconnect();

        bool m_disconnected;

        broker* m_broker;

    };

    class scribe : public network::stream_manager, public servant {

        typedef servant super;

     public:

        ~scribe();

        scribe(broker* parent, actor::connection_handle hdl);

        virtual void configure_read(receive_policy::config config) = 0;

        virtual buffer_type& wr_buf() = 0;

        virtual void flush() = 0;

        inline actor::connection_handle id() const {
            return read_msg().handle;
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

        actor::message disconnect_message() override;

        void consume(const void* data, size_t num_bytes) override;

        actor::message m_read_msg;

    };

    class doorman : public network::acceptor_manager, public servant {

        typedef servant super;

     public:

        ~doorman();

        doorman(broker* parent, actor::accept_handle hdl);

        inline actor::accept_handle id() const {
            return accept_msg().source;
        }

        actor::message disconnect_message() override;

        void io_failure(network::operation op, const std::string& msg) override;

     protected:

        inline actor::new_connection_msg& accept_msg() {
            return m_accept_msg.get_as_mutable<actor::new_connection_msg>(0);
        }

        inline const actor::new_connection_msg& accept_msg() const {
            return m_accept_msg.get_as<actor::new_connection_msg>(0);
        }

        actor::message m_accept_msg;

    };

    class continuation;

    // ... and some helpers need friendship
    friend class scribe;
    friend class doorman;
    friend class continuation;

    friend broker_ptr init_and_launch(broker_ptr);

    ~broker();

    /**
     * @brief Modifies the receive policy for given connection.
     * @param hdl Identifies the affected connection.
     * @param config Contains the new receive policy.
     */
    void configure_read(const actor::connection_handle& hdl,
                        receive_policy::config config);

    /**
     * @brief Returns the write buffer for given connection.
     */
    buffer_type& wr_buf(const actor::connection_handle& hdl);

    /**
     * @brief Sends the content of the buffer for given connection.
     */
    void flush(const actor::connection_handle& hdl);

    /**
     * @brief Returns the number of open connections.
     */
    inline size_t num_connections() const {
        return m_scribes.size();
    }

    /** @cond PRIVATE */

    template<typename F, typename... Ts>
    actor::actor fork(F fun, actor::connection_handle hdl, Ts&&... vs) {
        auto f = std::bind(std::move(fun),
                           std::placeholders::_1,
                           hdl,
                           std::forward<Ts>(vs)...);
        // transform to STD function here, because GCC is unable
        // to select proper overload otherwise ...
        typedef decltype(f((broker*) nullptr)) fres;
        std::function<fres(broker*)> stdfun{std::move(f)};
        return this->fork_impl(std::move(stdfun), hdl);
    }

    template<class Socket>
    actor::connection_handle add_connection(Socket sock) {
        class impl : public scribe {

            using super = scribe;

         public:

            impl(broker* parent, Socket&& s)
            : super(parent,
                    actor::connection_handle::from_int(s.native_handle()))
            , m_stream(s.get_io_service()) {
                m_stream.init(std::move(s));
            }

            void configure_read(receive_policy::config config) override {
                m_stream.configure_read(config);
            }

            buffer_type& wr_buf() override {
                return m_stream.wr_buf();
            }

            buffer_type& rd_buf() override {
                return m_stream.rd_buf();
            }

            void stop_reading() override {
                m_stream.stop_reading();
            }

            void flush() override {
                m_stream.flush(this);
            }

            void launch() override {
                m_stream.start(this);
            }

            network::stream<Socket> m_stream;

        };
        intrusive_ptr<impl> ptr{new impl{this, std::move(sock)}};
        m_scribes.emplace(ptr->id(), ptr);
        if (initialized()) ptr->launch();
        return ptr->id();
    }

    template<class SocketAcceptor>
    actor::accept_handle add_acceptor(SocketAcceptor fd) {
        class impl : public doorman {

            using super = doorman;

         public:

            impl(broker* parent, SocketAcceptor&& s)
            : super(parent,
                    actor::accept_handle::from_int(s.native_handle()))
            , m_acceptor(s.get_io_service()) {
                m_acceptor.init(std::move(s));
            }

            void new_connection() override {
                accept_msg().handle = m_broker->add_connection(std::move(m_acceptor.accepted_socket()));
                m_broker->invoke_message({}, m_accept_msg);
            }

            void stop_reading() override {
                m_acceptor.stop();
            }

            void launch() override {
                m_acceptor.start(this);
            }

            network::acceptor<SocketAcceptor> m_acceptor;

        };
        intrusive_ptr<impl> ptr{new impl{this, std::move(fd)}};
        m_doormen.emplace(ptr->id(), ptr);
        if (initialized()) ptr->launch();
        return ptr->id();
    }

    void enqueue(actor::msg_hdr_cref,
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

 protected:

    broker();

    void cleanup(uint32_t reason) override;

    typedef intrusive_ptr<scribe> scribe_pointer;

    typedef intrusive_ptr<doorman> doorman_pointer;

    bool initialized() const;

    /** @endcond */

    virtual actor::behavior make_behavior() = 0;

 private:

    // throws on error
    scribe& by_id(const actor::connection_handle& hdl);

    actor::actor fork_impl(std::function<void (broker*)> fun,
                           actor::connection_handle hdl);

    actor::actor fork_impl(std::function<actor::behavior (broker*)> fun,
                           actor::connection_handle hdl);

    static broker_ptr from_impl(std::function<void (broker*)> fun);

    static broker_ptr from_impl(std::function<actor::behavior (broker*)> fun);

    void invoke_message(actor::msg_hdr_cref hdr,
                        actor::message& msg);

    bool invoke_message_from_cache();

    void erase_io(int id);

    void erase_acceptor(int id);

    std::map<actor::accept_handle, doorman_pointer> m_doormen;
    std::map<actor::connection_handle, scribe_pointer> m_scribes;

    actor::policy::not_prioritizing  m_priority_policy;
    actor::policy::sequential_invoke m_invoke_policy;

    bool m_initialized;

};

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_BROKER_HPP
