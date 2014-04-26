#include <atomic>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <type_traits>

#include "test.hpp"
#include "boost/actor/detail/cs_thread.hpp"
#include "boost/actor/detail/yield_interface.hpp"

using namespace boost::actor;
using namespace boost::actor::detail;

struct pseudo_worker {

    int m_count;
    bool m_blocked;

    pseudo_worker() : m_count(0), m_blocked(true) { }

    void run [[noreturn]] () {
        for (;;) {
            if (m_blocked) {
                yield(yield_state::blocked);
            }
            else {
                ++m_count;
                yield(m_count < 10 ? yield_state::ready : yield_state::done);
            }
        }
    }

};

void coroutine [[noreturn]] (void* worker) {
    reinterpret_cast<pseudo_worker*>(worker)->run();
}

int main() {
    BOOST_ACTOR_TEST(test_yield_interface);
#   ifdef BOOST_ACTOR_DISABLE_CONTEXT_SWITCHING
    BOOST_ACTOR_PRINT("WARNING: context switching was explicitly "
               "disabled by defining BOOST_ACTOR_DISABLE_CONTEXT_SWITCHING");
#   else
    cs_thread fself;
    pseudo_worker worker;
    cs_thread fcoroutine(coroutine, &worker);
    yield_state ys;
    int i = 0;
    do {
        if (i == 2) worker.m_blocked = false;
        ys = call(&fcoroutine, &fself);
        ++i;
    }
    while (ys != yield_state::done && i < 12);
    BOOST_ACTOR_CHECK_EQUAL(to_string(ys), "yield_state::done");
    BOOST_ACTOR_CHECK_EQUAL(worker.m_count, 10);
    BOOST_ACTOR_CHECK_EQUAL(i, 12);
#   endif
    return BOOST_ACTOR_TEST_RESULT();
}
