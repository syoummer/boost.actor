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


#ifndef tcp_PEER_ACCEPTOR_HPP
#define tcp_PEER_ACCEPTOR_HPP

#include "boost/actor/actor.hpp"

#include "boost/actor_io/continuable.hpp"
#include "boost/actor_io/tcp_acceptor.hpp"

namespace boost {
namespace actor_io {

class peer_acceptor : public continuable {

    typedef continuable super;

 public:

    typedef std::set<std::string> string_set;

    continue_reading_result continue_reading() override;

    peer_acceptor(middleman* parent,
                  acceptor_uptr ptr,
                  const actor::actor_addr& published_actor,
                  string_set signatures);

    inline const actor::actor_addr& published_actor() const { return m_aa; }

    void dispose() override;

    void io_failed(event_bitmask) override;

 private:

    middleman*          m_parent;
    acceptor_uptr       m_ptr;
    actor::actor_addr   m_aa;
    string_set          m_sigs;

};

} // namespace actor_io
} // namespace boost

#endif // tcp_PEER_ACCEPTOR_HPP
