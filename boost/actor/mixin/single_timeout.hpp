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


#ifndef BOOST_ACTOR_MIXIN_SINGLE_TIMEOUT_HPP
#define BOOST_ACTOR_MIXIN_SINGLE_TIMEOUT_HPP

#include "boost/actor/message.hpp"
#include "boost/actor/duration.hpp"
#include "boost/actor/system_messages.hpp"

namespace boost {
namespace actor {
namespace mixin {

/**
 * @brief Mixin for actors using a non-nestable message processing.
 */
template<class Base, class Subtype>
class single_timeout : public Base {

    typedef Base super;

 public:

    typedef single_timeout combined_type;

    template <typename... Ts>
    single_timeout(Ts&&... args)
            : super(std::forward<Ts>(args)...), m_has_timeout(false)
            , m_timeout_id(0) { }

    void request_timeout(const duration& d) {
        if (d.valid()) {
            m_has_timeout = true;
            auto tid = ++m_timeout_id;
            auto msg = make_message(timeout_msg{tid});
            if (d.is_zero()) {
                // immediately enqueue timeout message if duration == 0s
                this->enqueue({this->address(), this}, std::move(msg),
                              this->m_host);
                //auto e = this->new_mailbox_element(this, std::move(msg));
                //this->m_mailbox.enqueue(e);
            }
            else this->delayed_send_tuple(this, d, std::move(msg));
        }
        else m_has_timeout = false;
    }

    inline bool waits_for_timeout(std::uint32_t timeout_id) const {
        return m_has_timeout && m_timeout_id == timeout_id;
    }

    inline bool is_active_timeout(std::uint32_t tid) const {
        return waits_for_timeout(tid);
    }

    inline bool has_active_timeout() const {
        return m_has_timeout;
    }

    inline void reset_timeout() {
        m_has_timeout = false;
    }

 protected:

    bool m_has_timeout;
    std::uint32_t m_timeout_id;

};

} // namespace mixin
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_MIXIN_SINGLE_TIMEOUT_HPP
