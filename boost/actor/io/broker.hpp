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
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * Distributed under the Boost Software License, Version 1.0. See             *
 * accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt  *
\******************************************************************************/


#ifndef BOOST_ACTOR_BROKER_HPP
#define BOOST_ACTOR_BROKER_HPP

#include <map>

#include "boost/actor/extend.hpp"
#include "boost/actor/local_actor.hpp"

#include "boost/actor/io/stream.hpp"
#include "boost/actor/io/buffer.hpp"
#include "boost/actor/io/acceptor.hpp"
#include "boost/actor/io/input_stream.hpp"
#include "boost/actor/io/tcp_acceptor.hpp"
#include "boost/actor/io/tcp_io_stream.hpp"
#include "boost/actor/io/output_stream.hpp"
#include "boost/actor/io/accept_handle.hpp"
#include "boost/actor/io/connection_handle.hpp"

#include "boost/actor/mixin/behavior_stack_based.hpp"

#include "boost/actor/policy/not_prioritizing.hpp"
#include "boost/actor/policy/sequential_invoke.hpp"

namespace boost {
namespace actor {
namespace io {

class broker;

typedef intrusive_ptr<broker> broker_ptr;

broker_ptr init_and_launch(broker_ptr);

/**
 * @brief A broker mediates between a libcppa-based actor system
 *        and other components in the network.
 * @extends local_actor
 */
class broker : public extend<local_actor>::
                      with<mixin::behavior_stack_based<behavior>::impl> {

    friend class policy::sequential_invoke;

    typedef combined_type super;

    // implementation relies on several helper classes ...
    class scribe;
    class servant;
    class doorman;
    class continuation;

    // ... and some helpers need friendship
    friend class scribe;
    friend class doorman;
    friend class continuation;

    friend broker_ptr init_and_launch(broker_ptr);

 public:

    ~broker();

    /**
     * @brief Used to configure {@link receive_policy()}.
     */
    enum policy_flag { at_least, at_most, exactly };

    /**
     * @brief Modifies the receive policy for this broker.
     * @param hdl Identifies the affected connection.
     * @param policy Sets the policy for given buffer size.
     * @param buffer_size Sets the minimal, maximum, or exact number of bytes
     *                    the middleman should read on this connection
     *                    before sending the next {@link new_data_msg}.
     */
    void receive_policy(const connection_handle& hdl,
                        broker::policy_flag policy,
                        size_t buffer_size);

    /**
     * @brief Sends data.
     */
    void write(const connection_handle& hdl, size_t num_bytes, const void* buf);

    /**
     * @brief Sends data.
     */
    void write(const connection_handle& hdl, const buffer& buf);

    /**
     * @brief Sends data.
     */
    void write(const connection_handle& hdl, buffer&& buf);

    /** @cond PRIVATE */

    template<typename F, typename... Ts>
    actor fork(F fun, connection_handle hdl, Ts&&... args) {
        auto f = std::bind(std::move(fun),
                           std::placeholders::_1,
                           hdl,
                           std::forward<Ts>(args)...);
        // transform to STD function here, because GCC is unable
        // to select proper overload otherwise ...
         typedef decltype(f((broker*) nullptr)) fres;
         std::function<fres(broker*)> stdfun{std::move(f)};
        return this->fork_impl(std::move(stdfun), hdl);
    }

    inline size_t num_connections() const {
        return m_io.size();
    }

    connection_handle add_connection(input_stream_ptr in, output_stream_ptr out);

    inline connection_handle add_connection(stream_ptr sptr) {
        return add_connection(sptr, sptr);
    }

    inline connection_handle add_tcp_connection(native_socket_type tcp_sockfd) {
        return add_connection(tcp_io_stream::from_sockfd(tcp_sockfd));
    }

    accept_handle add_acceptor(acceptor_uptr ptr);

    inline accept_handle add_tcp_acceptor(native_socket_type tcp_sockfd) {
        return add_acceptor(tcp_acceptor::from_sockfd(tcp_sockfd));
    }

    void enqueue(msg_hdr_cref, message, execution_unit*) override;

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
        return from(std::bind(fun, std::placeholders::_1,
                              std::forward<T>(v),
                              std::forward<Ts>(vs)...));
    }

 protected:

    broker();

    void cleanup(std::uint32_t reason) override;

    typedef std::unique_ptr<broker::scribe> scribe_pointer;

    typedef std::unique_ptr<broker::doorman> doorman_pointer;

    bool initialized() const;

    /** @endcond */

    virtual behavior make_behavior() = 0;

 private:

    actor fork_impl(std::function<void (broker*)> fun,
                    connection_handle hdl);

    actor fork_impl(std::function<behavior (broker*)> fun,
                    connection_handle hdl);

    static broker_ptr from_impl(std::function<void (broker*)> fun);

    static broker_ptr from_impl(std::function<behavior (broker*)> fun);

    void invoke_message(msg_hdr_cref hdr, message msg);

    bool invoke_message_from_cache();

    void erase_io(int id);

    void erase_acceptor(int id);

    std::map<accept_handle, doorman_pointer> m_accept;
    std::map<connection_handle, scribe_pointer> m_io;

    policy::not_prioritizing  m_priority_policy;
    policy::sequential_invoke m_invoke_policy;

    bool m_initialized;

};

} // namespace io
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_BROKER_HPP
