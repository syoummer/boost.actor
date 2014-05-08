#include "boost/actor/config.hpp"

#include <map>
#include <list>
#include <mutex>
#include <iostream>
#include <algorithm>

#include "test.hpp"

#include "boost/actor/on.hpp"
#include "boost/actor/all.hpp"
#include "boost/actor/actor.hpp"
#include "boost/actor/abstract_group.hpp"
#include "boost/actor/ref_counted.hpp"
#include "boost/intrusive_ptr.hpp"

using std::cout;
using std::endl;

using std::string;
using namespace boost::actor;

void testee(event_based_actor* self, int current_value, int final_result) {
    self->become(
        on_arg_match >> [=](int result) {
            auto next = result + current_value;
            if (next >= final_result) self->quit();
            else testee(self, next, final_result);
        },
        after(std::chrono::seconds(2)) >> [=] {
            BOOST_ACTOR_UNEXPECTED_TOUT();
            self->quit(exit_reason::user_shutdown);
        }
    );
}

int main() {
    BOOST_ACTOR_TEST(test_local_group);
    /*
    auto foo_group = group::get("local", "foo");
    auto master = spawn_in_group(foo_group, testee, 0, 10);
    for (int i = 0; i < 5; ++i) {
        // spawn five workers and let them join local/foo
        spawn_in_group(foo_group, [master] {
            become(on_arg_match >> [master](int v) {
                send(master, v);
                self->quit();
            });
        });
    }
    send(foo_group, 2);
    await_all_actors_done();
    shutdown();
    */
    return BOOST_ACTOR_TEST_RESULT();
}
