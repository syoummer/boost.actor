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


#include <memory>
#include <iostream>

#include "test.hpp"
#include "boost/actor/all.hpp"
#include "boost/actor_io/all.hpp"

using namespace std;
using namespace boost::actor;
using namespace boost::actor_io;

void ping(event_based_actor* self, size_t num_pings) {
    BOOST_ACTOR_PRINT("num_pings: " << num_pings);
    auto count = std::make_shared<size_t>(0);
    self->become (
        on(atom("kickoff"), arg_match) >> [=](const actor& pong) {
            BOOST_ACTOR_CHECKPOINT();
            self->send(pong, atom("ping"), 1);
            self->become (
                on(atom("pong"), arg_match)
                >> [=](int value) -> std::tuple<atom_value, int> {
                    if (++*count >= num_pings) {
                        BOOST_ACTOR_PRINT("received " << num_pings
                                          << " pings, call self->quit");
                        self->quit();
                    }
                    return std::make_tuple(atom("ping"), value + 1);
                },
                others() >> BOOST_ACTOR_UNEXPECTED_MSG_CB(self)
            );
        },
        others() >> BOOST_ACTOR_UNEXPECTED_MSG_CB(self)
    );
}

void pong(event_based_actor* self) {
    BOOST_ACTOR_CHECKPOINT();
    self->become  (
        on(atom("ping"), arg_match)
        >> [=](int value) -> std::tuple<atom_value, int> {
            BOOST_ACTOR_CHECKPOINT();
            self->monitor(self->last_sender());
            // set next behavior
            self->become (
                on(atom("ping"), arg_match) >> [](int val) {
                    return std::make_tuple(atom("pong"), val);
                },
                on_arg_match >> [=](const down_msg& dm) {
                    BOOST_ACTOR_PRINT("received down_msg{" << dm.reason << "}");
                    self->quit(dm.reason);
                },
                others() >> BOOST_ACTOR_UNEXPECTED_MSG_CB(self)
            );
            // reply to 'ping'
            return std::make_tuple(atom("pong"), value);
        },
        others() >> BOOST_ACTOR_UNEXPECTED_MSG_CB(self)
    );
}

void peer_fun(broker* self, const actor& buddy) {
    BOOST_ACTOR_CHECKPOINT();
    BOOST_ACTOR_CHECK(self != nullptr);
    BOOST_ACTOR_CHECK(buddy != invalid_actor);
    self->monitor(buddy);
    // assume exactly one connection
    auto cons = self->connections();
    if (cons.size() != 1) {
        cerr << "expected 1 connection, found " << cons.size() << endl;
        throw std::logic_error("num_connections() != 1");
    }
    auto hdl = cons.front();
    self->configure_read(hdl, receive_policy::exactly(  sizeof(atom_value)
                                                      + sizeof(int)));
    auto write = [=](atom_value type, int value) {
        BOOST_ACTOR_LOGF_DEBUG("write: " << value);
        auto& buf = self->wr_buf(hdl);
        auto first = reinterpret_cast<char*>(&type);
        buf.insert(buf.end(), first, first + sizeof(atom_value));
        first = reinterpret_cast<char*>(&value);
        buf.insert(buf.end(), first, first + sizeof(int));
        self->flush(hdl);
    };
    self->become (
        [=](const connection_closed_msg&) {
            BOOST_ACTOR_PRINT("received connection_closed_msg");
            self->quit();
        },
        [=](const new_data_msg& msg) {
            BOOST_ACTOR_PRINT("received new_data_msg");
            atom_value type;
            int value;
            memcpy(&type, msg.buf.data(), sizeof(atom_value));
            memcpy(&value, msg.buf.data() + sizeof(atom_value), sizeof(int));
            self->send(buddy, type, value);
        },
        on(atom("ping"), arg_match) >> [=](int value) {
            BOOST_ACTOR_PRINT("received ping{" << value << "}");
            write(atom("ping"), value);
        },
        on(atom("pong"), arg_match) >> [=](int value) {
            BOOST_ACTOR_PRINT("received pong{" << value << "}");
            write(atom("pong"), value);
        },
        [=](const down_msg& dm) {
            BOOST_ACTOR_PRINT("received down_msg");
            if (dm.source == buddy) self->quit(dm.reason);
        },
        others() >> BOOST_ACTOR_UNEXPECTED_MSG_CB(self)
    );
}

behavior peer_acceptor_fun(broker* self, const actor& buddy) {
    BOOST_ACTOR_CHECKPOINT();
    return {
        [=](const new_connection_msg& msg) {
            BOOST_ACTOR_CHECKPOINT();
            BOOST_ACTOR_PRINT("received new_connection_msg");
            self->fork(peer_fun, msg.handle, buddy);
            self->quit();
        },
        others() >> BOOST_ACTOR_UNEXPECTED_MSG_CB(self)
    };
}

int main(int argc, char** argv) {
    BOOST_ACTOR_TEST(test_broker);
    message_builder{argv + 1, argv + argc}.apply({
        on("-c", arg_match) >> [&](const std::string& portstr) {
            auto port = static_cast<uint16_t>(std::stoi(portstr));
            auto p = spawn(ping, 10);
            BOOST_ACTOR_CHECKPOINT();
            auto cl = spawn_functor(nullptr,
                                    [=](broker* bro) {
                                        bro->add_connection(network::new_ipv4_connection("localhost", port));
                                    },
                                    peer_fun, p);
            BOOST_ACTOR_CHECKPOINT();
            anon_send(p, atom("kickoff"), cl);
            BOOST_ACTOR_CHECKPOINT();
        },
        on() >> [&] {
            auto p = spawn(pong);
            uint16_t port = 4242;
            bool done = false;
            while (!done) {
                try {
                    spawn_functor(nullptr,
                                  [=](broker* bro) {
                                      bro->add_acceptor(network::new_ipv4_acceptor(port));
                                  },
                                  peer_acceptor_fun,
                                  p);
                    BOOST_ACTOR_CHECKPOINT();
                    ostringstream oss;
                    oss << argv[0] << " -c " << port << to_dev_null;
                    thread child{[&oss] {
                        BOOST_ACTOR_LOGC_TRACE("NONE", "main$thread_launcher", "");
                        auto cmdstr = oss.str();
                        if (system(cmdstr.c_str()) != 0) {
                            BOOST_ACTOR_PRINTERR("FATAL: command failed: " << cmdstr);
                            abort();
                        }
                    }};
                    BOOST_ACTOR_CHECKPOINT();
                    child.join();
                    done = true;
                }
                catch (bind_failure&) {
                    // try next port
                    ++port;
                }
            }
        },
        others() >> [&] {
            cerr << "usage: " << argv[0] << " [-c PORT]" << endl;
        }
    });
    BOOST_ACTOR_CHECKPOINT();
    await_all_actors_done();
    BOOST_ACTOR_CHECKPOINT();
    shutdown();
    return BOOST_ACTOR_TEST_RESULT();
}
