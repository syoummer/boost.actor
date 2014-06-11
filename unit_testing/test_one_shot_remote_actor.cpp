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


#include "test.hpp"

#include "boost/actor_io/all.hpp"

using namespace std;
using namespace boost::actor;
using namespace boost::actor_io;

void run_client(uint16_t port, bool add_monitor) {
    BOOST_ACTOR_LOGF_INFO("run in client mode");
    scoped_actor self;
    self->send(remote_actor("localhost", port), atom("Hey"));
    self->receive(
        on(atom("Hey")) >> [&] {
            BOOST_ACTOR_CHECKPOINT();
            if (add_monitor) self->monitor(self->last_sender());
            self->send_exit(self->last_sender(),
                            exit_reason::user_shutdown);
        },
        after(std::chrono::seconds(10)) >> BOOST_ACTOR_UNEXPECTED_TOUT_CB()
    );
    if (add_monitor) {
        self->receive(
            [&](down_msg& msg) {
                BOOST_ACTOR_CHECK_EQUAL(msg.reason,
                                        exit_reason::user_shutdown);
            },
            after(std::chrono::seconds(10)) >> BOOST_ACTOR_UNEXPECTED_TOUT_CB()
        );
    }
}

void reflector(event_based_actor* self, int num_exits = 0) {
    self->trap_exit(true);
    self->become (
        [=](exit_msg& msg) {
            BOOST_ACTOR_PRINT("received exit message");
            if (num_exits > 0) self->quit(msg.reason);
            else reflector(self, num_exits + 1);
        },
        others() >> [=] {
            BOOST_ACTOR_PRINT("reflect");
            return self->last_dequeued();
        }
    );
}

void test_one_shot_remote_actor(const char* app_path, bool run_remote_actor) {
    auto serv = spawn(reflector, 0);
    uint16_t port = 4242;
    bool success = false;
    do {
        try {
            publish(serv, port, "127.0.0.1");
            success = true;
            BOOST_ACTOR_PRINT("running on port " << port);
            BOOST_ACTOR_LOGF_INFO("running on port " << port);
        }
        catch (bind_failure&) {
            // try next port
            ++port;
        }
    }
    while (!success);
    thread child;
    ostringstream oss;
    if (run_remote_actor) {
        oss << app_path << " -c " << port << to_dev_null;
        // execute client_part() in a separate process,
        // connected via localhost socket
        child = thread([&oss]() {
            BOOST_ACTOR_LOGC_TRACE("NONE", "main$thread_launcher", "");
            string cmdstr = oss.str();
            if (system(cmdstr.c_str()) != 0) {
                BOOST_ACTOR_PRINTERR("FATAL: command \"" << cmdstr << "\" failed!");
                abort();
            }
        });
    }
    else { BOOST_ACTOR_PRINT("actor published at port " << port); }
    BOOST_ACTOR_CHECKPOINT();
    if (run_remote_actor) child.join();
}

int main(int argc, char** argv) {
    cout << "this node is: "
         << to_string(boost::actor::detail::singletons::get_node_id())
         << endl;
    message_builder{argv + 1, argv + argc}.apply({
        on("-c", spro<uint16_t>) >> [](uint16_t port) {
            BOOST_ACTOR_LOGF_INFO("run in client mode");
            try {
                run_client(port, false);
                run_client(port, true);
            }
            catch (std::exception& e) {
                BOOST_ACTOR_PRINT("exception: " << e.what());
            }
        },
        on("-s") >> [&] {
            BOOST_ACTOR_PRINT("don't run remote actor (server mode)");
            test_one_shot_remote_actor(argv[0], false);
        },
        on() >> [&] {
            test_one_shot_remote_actor(argv[0], true);
        },
        others() >> [&] {
            BOOST_ACTOR_PRINTERR("usage: " << argv[0] << " [-s|-c PORT]");
        }
    });
    await_all_actors_done();
    shutdown();
    return BOOST_ACTOR_TEST_RESULT();
}
