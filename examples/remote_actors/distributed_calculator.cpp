/******************************************************************************\
 * This program is a distributed version of the math_actor example.           *
 * Client and server use a stateless request/response protocol and the client *
 * is failure resilient by using a FIFO request queue.                        *
 * The client auto-reconnects and also allows for server reconfiguration.     *
 *                                                                            *
 * Run server at port 4242:                                                   *
 * - ./build/bin/distributed_math_actor -s -p 4242                            *
 *                                                                            *
 * Run client at the same host:                                               *
 * - ./build/bin/distributed_math_actor -c -p 4242                            *
\******************************************************************************/

#include <regex>
#include <vector>
#include <string>
#include <sstream>
#include <cassert>
#include <iostream>
#include <functional>

#include "boost/none.hpp"
#include "boost/program_options.hpp"

#include "boost/actor/cppa.hpp"

using boost::none;
using boost::optional;

using namespace std;
using namespace boost::actor;
using namespace boost::actor::placeholders;
using namespace boost::program_options;

// our "service"
void calculator(event_based_actor* self) {
    self->become (
        on(atom("plus"), arg_match) >> [](int a, int b) -> any_tuple {
            return make_any_tuple(atom("result"), a + b);
        },
        on(atom("minus"), arg_match) >> [](int a, int b) -> any_tuple {
            return make_any_tuple(atom("result"), a - b);
        },
        on(atom("quit")) >> [=] {
            self->quit();
        }
    );
}

inline string trim(std::string s) {
    auto not_space = [](char c) { return !isspace(c); };
    // trim left
    s.erase(s.begin(), find_if(s.begin(), s.end(), not_space));
    // trim right
    s.erase(find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
}

void client_bhvr(event_based_actor* self, const string& host, uint16_t port, const actor& server) {
    // recover from sync failures by trying to reconnect to server
    if (!self->has_sync_failure_handler()) {
        self->on_sync_failure([=] {
            aout(self) << "*** lost connection to " << host
                       << ":" << port << endl;
            client_bhvr(self, host, port, invalid_actor);
        });
    }
    // connect to server if needed
    if (!server) {
        aout(self) << "*** try to connect to " << host << ":" << port << endl;
        try {
            auto new_serv = remote_actor(host, port);
            self->monitor(new_serv);
            aout(self) << "reconnection succeeded" << endl;
            client_bhvr(self, host, port, new_serv);
            return;
        }
        catch (std::exception&) {
            aout(self) << "connection failed, try again in 3s" << endl;
            self->delayed_send(self, chrono::seconds(3), atom("reconnect"));
        }
    }
    self->become (
        on_arg_match.when(_x1.in({atom("plus"), atom("minus")}) && gval(server) != invalid_actor) >> [=](atom_value op, int lhs, int rhs) {
            self->sync_send_tuple(server, self->last_dequeued()).then(
                on(atom("result"), arg_match) >> [=](int result) {
                    aout(self) << lhs << " "
                               << to_string(op) << " "
                               << rhs << " = "
                               << result << endl;
                }
            );
        },
        on_arg_match >> [=](const down_msg&) {
            aout(self) << "*** server down, try to reconnect ..." << endl;
            client_bhvr(self, host, port, invalid_actor);
        },
        on(atom("rebind"), arg_match) >> [=](const string& nhost, uint16_t nport) {
            aout(self) << "*** rebind to new server: "
                       << nhost << ":" << nport << endl;
            client_bhvr(self, nhost, nport, invalid_actor);
        },
        on(atom("reconnect")) >> [=] {
            client_bhvr(self, host, port, invalid_actor);
        }
    );
}

void client_repl(const string& host, uint16_t port) {
    // keeps track of requests and tries to reconnect on server failures
    cout << "Usage:\n"
            "quit                   Quit the program\n"
            "<x> + <y>              Calculate <x>+<y> and print result\n"
            "<x> - <y>              Calculate <x>-<y> and print result\n"
            "connect <host> <port>  Reconfigure server"
         << endl << endl;
    string line;
    auto client = spawn(client_bhvr, host, port, invalid_actor);
    std::regex connect_rx{"connect (.+) ([0-9]+)"};
    std::smatch base_match;
    auto toint = [](const string& str) -> optional<int> {
        try { return {std::stoi(str)}; }
        catch (std::exception&) {
            cout << "\"" << str << "\" is not an integer" << endl;
            return none;
        }
    };
    while (getline(cin, line)) {
        line = trim(std::move(line)); // ignore leading and trailing whitespaces
        if (line == "quit") {
            // force client to quit
            anon_send_exit(client, exit_reason::user_shutdown);
            return;
        }
        if (std::regex_match(line, base_match, connect_rx) && base_match.size() == 3) {
            auto nhost = base_match[1].str();
            try {
                auto lport = std::stoul(base_match[2].str());
                if (lport < std::numeric_limits<uint16_t>::max()) {
                    anon_send(client, atom("rebind"), move(nhost),
                              static_cast<uint16_t>(lport));
                }
                else {
                    cout << lport << " is not a valid port" << endl;
                }
            }
            catch (std::exception&) {
                cout << "invalid port declaration" << endl;
            }
        }
        else {
            bool success = false;
            auto first = begin(line);
            auto last = end(line);
            auto pos = find_if(first, last, [](char c) { return c == '+' || c == '-'; });
            if (pos != last) {
                auto lsub = trim(string(first, pos));
                auto rsub = trim(string(pos + 1, last));
                auto lhs = toint(lsub);
                auto rhs = toint(rsub);
                if (lhs && rhs) {
                    auto op = (*pos == '+') ? atom("plus") : atom("minus");
                    anon_send(client, op, *lhs, *rhs);
                }
            }
            else if (!success) {
                cout << "*** invalid format; usage: <x> [+|-] <y>" << endl;
            }
        }
    }
}

int main(int argc, char** argv) {
    string mode;
    string host;
    string group_id;
    uint16_t port = 0;
    options_description desc("Allowed options");
    desc.add_options()
        ("port,p", value<uint16_t>(&port), "set name")
        ("host,H", value<string>(&host)->default_value("localhost"), "set host")
        ("server,s", value<string>(&group_id), "run in server mode")
        ("client,c", value<string>(&group_id), "run in client mode")
        ("help,h", "print help")
    ;
    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        cout << desc << endl;
        return 1;
    }
    if (vm.count("server")) {
        mode = "server";
    }
    if (vm.count("client")) {
        if (!mode.empty()) {
            cerr << "*** cannot run in both client and server mode" << endl;
            cout << desc << endl;
            return 2;
        }
        mode = "client";
    }
    if (!vm.count("port")) {
        cerr << "*** no port specified" << endl;
        cout << desc << endl;
        return 2;
    }
    if (mode == "server") {
        try {
            // try to publish math actor at given port
            publish(spawn(calculator), port);
        }
        catch (std::exception& e) {
            cerr << "*** unable to publish math actor at port " << port << "\n"
                 << to_verbose_string(e) // prints exception type and e.what()
                 << endl;
        }
    }
    else {
        if (host.empty()) host = "localhost";
        client_repl(host, port);
    }
    await_all_actors_done();
    shutdown();
    return 0;
}
