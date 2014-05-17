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


#ifndef BOOST_ACTOR_MESSAGE_QUEUE_HPP
#define BOOST_ACTOR_MESSAGE_QUEUE_HPP

#include <vector>

#include "boost/actor/message.hpp"
#include "boost/actor/ref_counted.hpp"
#include "boost/actor/message_header.hpp"

namespace boost {
namespace actor {
namespace io {

class default_message_queue : public ref_counted {

 public:

    typedef std::pair<message_header, message> value_type;

    typedef value_type& reference;

    ~default_message_queue();

    template<typename... Ts>
    void emplace(Ts&&... args) {
        m_impl.emplace_back(std::forward<Ts>(args)...);
    }

    inline bool empty() const { return m_impl.empty(); }

    inline value_type pop() {
        value_type result(std::move(m_impl.front()));
        m_impl.erase(m_impl.begin());
        return result;
    }

 private:

    std::vector<value_type> m_impl;

};

typedef intrusive_ptr<default_message_queue> default_message_queue_ptr;

} // namespace io
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_MESSAGE_QUEUE_HPP
