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


#ifndef MIDDLEMAN_HPP
#define MIDDLEMAN_HPP

#include <map>
#include <vector>
#include <memory>
#include <thread>

#include "boost/asio.hpp"

#include "boost/actor/fwd.hpp"
#include "boost/actor/node_id.hpp"
#include "boost/actor/actor_namespace.hpp"
#include "boost/actor/detail/singletons.hpp"

#include "boost/actor_io/broker.hpp"
#include "boost/actor_io/network.hpp"

namespace boost {
namespace actor_io {

/**
 * @brief Manages brokers.
 */
class middleman : public actor::detail::abstract_singleton {

    friend class boost::actor::detail::singletons;

 public:

    using node_id = actor::node_id;

    /**
     * @brief Get middleman instance.
     */
    static middleman* instance();

    ~middleman();

    /**
     * @brief Returns the broker associated with @p name.
     */
    template<class Impl>
    intrusive_ptr<Impl> get_named_broker(actor::atom_value name);

    /**
     * @brief Adds @p bptr to the list of known brokers.
     */
    void add_broker(broker_ptr bptr);

    /**
     * @brief Runs @p fun in the event loop of the middleman.
     * @note This member function is thread-safe.
     */
    template<typename F>
    void run_later(F fun) {
        m_backend.dispatch(fun);
    }

    /**
     * @brief Returns the IO backend used by this middleman.
     */
    inline boost::asio::io_service& backend() {
        return m_backend;
    }

    /** @cond PRIVATE */

    // destroys uninitialized instances
    void dispose() override;

    // destroys an initialized singleton
    void destroy() override;

    // initializes a singleton
    void initialize() override;

    /** @endcond */

 private:

    middleman();

    network::multiplexer             m_backend;    // networking backend
    network::supervisor*             m_supervisor; // keeps backend busy

    std::thread                      m_thread;     // runs the backend

    std::map<actor::atom_value, broker_ptr> m_named_brokers;

    std::set<broker_ptr> m_brokers;

};

template<class Impl>
intrusive_ptr<Impl> middleman::get_named_broker(actor::atom_value name) {
    auto i = m_named_brokers.find(name);
    if (i != m_named_brokers.end()) return static_cast<Impl*>(i->second.get());
    intrusive_ptr<Impl> result{new Impl};
    result->launch(true, nullptr);
    m_named_brokers.emplace(name, result);
    return result;
}

} // namespace actor_io
} // namespace boost

#endif // MIDDLEMAN_HPP
