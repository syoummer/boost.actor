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


#include "boost/actor/all.hpp"

#include "boost/actor_io/publish_local_groups.hpp"

namespace boost {
namespace actor_io {

using namespace actor;

namespace {
struct group_nameserver : event_based_actor {
    behavior make_behavior() override {
        return (
            on(atom("GET_GROUP"), arg_match) >> [](const std::string& name) {
                return make_message(atom("GROUP"), group::get("local", name));
            },
            on(atom("SHUTDOWN")) >> [=] {
                quit();
            }
        );
    }
};
} // namespace <anonymous>

void publish_local_groups(std::uint16_t port, const char* addr) {
    auto gn = spawn<group_nameserver, hidden>();
    try {
        publish(gn, port, addr);
    }
    catch (std::exception&) {
        gn->enqueue({invalid_actor_addr, nullptr},
                    make_message(atom("SHUTDOWN")),
                    nullptr);
        throw;
    }
}

} // namespace actor_io
} // namespace boost
