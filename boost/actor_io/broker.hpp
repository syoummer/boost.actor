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

#include "boost/actor_io/stream.hpp"
#include "boost/actor_io/broker.hpp"
#include "boost/actor_io/acceptor.hpp"
#include "boost/actor_io/input_stream.hpp"
#include "boost/actor_io/tcp_acceptor.hpp"
#include "boost/actor_io/tcp_io_stream.hpp"
#include "boost/actor_io/output_stream.hpp"

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

    typedef std::vector<char> charbuf;

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
    void receive_policy(const boost::actor::connection_handle& hdl,
                        broker::policy_flag policy,
                        size_t buffer_size);

    /**
     * @brief Sends data.
     */
    void write(const boost::actor::connection_handle& hdl,
               size_t num_bytes,
               const void* buf);

    /**
     * @brief Sends data.
     */
    void write(const boost::actor::connection_handle& hdl, const charbuf& buf);

    /** @cond PRIVATE */

    template<typename F, typename... Ts>
    actor::actor fork(F fun, boost::actor::connection_handle hdl, Ts&&... vs) {
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

    inline size_t num_connections() const {
        return m_io.size();
    }

    boost::actor::connection_handle add_connection(input_stream_ptr in,
                                                   output_stream_ptr out);

    inline boost::actor::connection_handle add_connection(stream_ptr sptr) {
        return add_connection(sptr, sptr);
    }

    inline boost::actor::connection_handle
    add_tcp_connection(native_socket_type tcp_sockfd) {
        return add_connection(tcp_io_stream::from_sockfd(tcp_sockfd));
    }

    boost::actor::accept_handle add_acceptor(acceptor_uptr ptr);

    inline boost::actor::accept_handle
    add_tcp_acceptor(native_socket_type tcp_sockfd) {
        return add_acceptor(tcp_acceptor::from_sockfd(tcp_sockfd));
    }

    void enqueue(boost::actor::msg_hdr_cref,
                 boost::actor::message,
                 boost::actor::execution_unit*) override;

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

    virtual actor::behavior make_behavior() = 0;

 private:

    boost::actor::actor fork_impl(std::function<void (broker*)> fun,
                                  boost::actor::connection_handle hdl);

    boost::actor::actor fork_impl(std::function<actor::behavior (broker*)> fun,
                                  boost::actor::connection_handle hdl);

    static broker_ptr from_impl(std::function<void (broker*)> fun);

    static broker_ptr from_impl(std::function<boost::actor::behavior (broker*)> fun);

    void invoke_message(boost::actor::msg_hdr_cref hdr,
                        boost::actor::message msg);

    bool invoke_message_from_cache();

    void erase_io(int id);

    void erase_acceptor(int id);

    std::map<boost::actor::accept_handle, doorman_pointer> m_accept;
    std::map<boost::actor::connection_handle, scribe_pointer> m_io;

    actor::policy::not_prioritizing  m_priority_policy;
    actor::policy::sequential_invoke m_invoke_policy;

    bool m_initialized;

};

} // namespace actor_io
} // namespace boost

#endif // BOOST_ACTOR_BROKER_HPP
