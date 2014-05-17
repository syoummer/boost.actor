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


#ifndef BOOST_ACTOR_MIXIN_MAILBOX_BASED_HPP
#define BOOST_ACTOR_MIXIN_MAILBOX_BASED_HPP

#include <type_traits>

#include "boost/actor/mailbox_element.hpp"

#include "boost/actor/detail/sync_request_bouncer.hpp"
#include "boost/actor/detail/single_reader_queue.hpp"

namespace boost {
namespace actor {
namespace mixin {

template<class Base, class Subtype>
class mailbox_based : public Base {

    typedef detail::disposer del;

 public:

    ~mailbox_based() {
        if (!m_mailbox.closed()) {
            detail::sync_request_bouncer f{this->exit_reason()};
            m_mailbox.close(f);
        }
    }

    template<typename... Ts>
    inline mailbox_element* new_mailbox_element(Ts&&... args) {
        return mailbox_element::create(std::forward<Ts>(args)...);
    }

    void cleanup(std::uint32_t reason) override {
        detail::sync_request_bouncer f{reason};
        m_mailbox.close(f);
        Base::cleanup(reason);
    }

 protected:

    typedef mailbox_based combined_type;

    typedef detail::single_reader_queue<mailbox_element, del> mailbox_type;

    template<typename... Ts>
    mailbox_based(Ts&&... args) : Base(std::forward<Ts>(args)...) { }

    mailbox_type m_mailbox;

};

} // namespace mixin
} // namespace actor
} // namespace boost

#endif //BOOST_ACTOR_MIXIN_MAILBOX_BASED_HPP
