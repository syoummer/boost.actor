#include <list>
#include <cstddef>

#include "test.hpp"
#include "boost/intrusive_ptr.hpp"
#include "boost/actor/ref_counted.hpp"
#include "boost/actor/detail/make_counted.hpp"

using boost::intrusive_ptr;
using namespace boost::actor;

namespace {

int class0_instances = 0;
int class1_instances = 0;

struct class0 : ref_counted {
    class0() { ++class0_instances; }

    virtual ~class0() { --class0_instances; }

    virtual class0* create() const { return new class0; }
};

struct class1 : class0 {
    class1() { ++class1_instances; }

    virtual ~class1() { --class1_instances; }

    virtual class1* create() const { return new class1; }
};

typedef intrusive_ptr<class0> class0_ptr;
typedef intrusive_ptr<class1> class1_ptr;

class0* get_test_rc() {
    return new class0;
}

class0_ptr get_test_ptr() {
    return get_test_rc();
}

} // namespace <anonymous>

int main() {
    // this test dosn't test thread-safety of intrusive_ptr
    // however, it is thread safe since it uses atomic operations only

    BOOST_ACTOR_TEST(test_intrusive_ptr);
    {
        auto p = detail::make_counted<class0>();
        BOOST_ACTOR_CHECK_EQUAL(class0_instances, 1);
        BOOST_ACTOR_CHECK(p->unique());
    }
    BOOST_ACTOR_CHECK_EQUAL(class0_instances, 0);
    {
        class0_ptr p;
        p = new class0;
        BOOST_ACTOR_CHECK_EQUAL(class0_instances, 1);
        BOOST_ACTOR_CHECK(p->unique());
    }
    BOOST_ACTOR_CHECK_EQUAL(class0_instances, 0);
    {
        class0_ptr p1;
        p1 = get_test_rc();
        class0_ptr p2 = p1;
        BOOST_ACTOR_CHECK_EQUAL(class0_instances, 1);
        BOOST_ACTOR_CHECK_EQUAL(p1->unique(), false);
    }
    BOOST_ACTOR_CHECK_EQUAL(class0_instances, 0);
    {
        std::list<class0_ptr> pl;
        pl.push_back(get_test_ptr());
        pl.push_back(get_test_rc());
        pl.push_back(pl.front()->create());
        BOOST_ACTOR_CHECK(pl.front()->unique());
        BOOST_ACTOR_CHECK_EQUAL(class0_instances, 3);
    }
    BOOST_ACTOR_CHECK_EQUAL(class0_instances, 0);
    {
        auto p1 = detail::make_counted<class0>();
        p1 = new class1;
        BOOST_ACTOR_CHECK_EQUAL(class0_instances, 1);
        BOOST_ACTOR_CHECK_EQUAL(class1_instances, 1);
        auto p2 = detail::make_counted<class1>();
        p1 = p2;
        BOOST_ACTOR_CHECK_EQUAL(class0_instances, 1);
        BOOST_ACTOR_CHECK_EQUAL(class1_instances, 1);
        BOOST_ACTOR_CHECK(p1 == p2);
    }
    BOOST_ACTOR_CHECK_EQUAL(class0_instances, 0);
    BOOST_ACTOR_CHECK_EQUAL(class1_instances, 0);

    return BOOST_ACTOR_TEST_RESULT();

}
