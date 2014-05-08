#include <thread>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <functional>

#include "test.hpp"
#include "boost/actor/all.hpp"

using namespace std;
using namespace boost::actor;

struct ping { std::int32_t value; };

bool operator==(const ping& lhs, const ping& rhs) {
    return lhs.value == rhs.value;
}

struct pong { std::int32_t value; };

bool operator==(const pong& lhs, const pong& rhs) {
    return lhs.value == rhs.value;
}

using server_type = typed_actor<
                        replies_to<ping>::with<pong>
                    >;

typedef typed_actor<> client_type;

server_type::behavior_type server() {
    return {
        [](const ping& p) -> pong {
            BOOST_ACTOR_CHECKPOINT();
            return pong{p.value};
        }
    };
}

void run_client(const char* host, std::uint16_t port) {
    // check whether invalid_argument is thrown
    // when trying to connect to get an untyped
    // handle to the server
    try {
        remote_actor(host, port);
    }
    catch (std::invalid_argument& e) {
        cout << e.what() << endl;
        BOOST_ACTOR_CHECKPOINT();
    }
    catch (std::exception& e) {
        cerr << "unexpected: " << e.what() << endl;
    }
    BOOST_ACTOR_CHECKPOINT();
    auto serv = typed_remote_actor<server_type::interface>(host, port);
    BOOST_ACTOR_CHECKPOINT();
    scoped_actor self;
    self->sync_send(serv, ping{42}).await(
        [](const pong& p) {
            BOOST_ACTOR_CHECK_EQUAL(p.value, 42);
        }
    );
    anon_send_exit(serv, exit_reason::user_shutdown);
}

std::uint16_t run_server() {
    auto ref = spawn_typed(server);
    uint16_t port = 4242;
    for (;;) {
        try {
            typed_publish(ref, port, "127.0.0.1");
            BOOST_ACTOR_LOGF_DEBUG("running on port " << port);
            return port;
        }
        catch (bind_failure&) {
            // try next port
            ++port;
        }
    }
}

int main(int argc, char** argv) {
    announce<ping>(&ping::value);
    announce<pong>(&pong::value);
    auto run_remote_actor = true;
    if (argc > 1) {
        if (strcmp(argv[1], "run_remote_actor=false") == 0) {
            BOOST_ACTOR_PRINT("don't run remote actor");
            run_remote_actor = false;
        }
        else {
            auto kvp = get_kv_pairs(argc, argv);
            if (kvp.count("port") == 0) {
                throw std::invalid_argument("no port given");
            }
            run_client("localhost",
                       static_cast<std::uint16_t>(std::stoi(kvp["port"])));
            BOOST_ACTOR_CHECKPOINT();
            await_all_actors_done();
            shutdown();
            return BOOST_ACTOR_TEST_RESULT();
        }
    }
    BOOST_ACTOR_CHECKPOINT();
    auto port = run_server();
    BOOST_ACTOR_CHECKPOINT();
    if (run_remote_actor) {
        BOOST_ACTOR_CHECKPOINT();
        thread child;
        ostringstream oss;
        oss << argv[0] << " run=remote_actor port=" << port << to_dev_null;
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
        BOOST_ACTOR_CHECKPOINT();
        child.join();
    }
    else {
        BOOST_ACTOR_PRINT("actor published at port " << port);
    }
    BOOST_ACTOR_CHECKPOINT();
    await_all_actors_done();
    shutdown();
    return BOOST_ACTOR_TEST_RESULT();
}
