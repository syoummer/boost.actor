#include <string>
#include <cstdint>
#include <typeinfo>
#include <type_traits>

#include "test.hpp"

#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/detail/int_list.hpp"
#include "boost/actor/detail/type_list.hpp"

#include "boost/actor/detail/demangle.hpp"

using std::cout;
using std::endl;
using std::is_same;

using namespace boost::actor;
using namespace boost::actor::detail;

template<typename T>
struct is_int : std::false_type { };

template<>
struct is_int<int> : std::true_type { };

int main() {

    BOOST_ACTOR_TEST(test_metaprogramming);

    typedef type_list<int, float, std::string> l1;
    typedef typename tl_reverse<l1>::type r1;

    BOOST_ACTOR_CHECK((is_same<int, tl_at<l1, 0>::type>::value));
    BOOST_ACTOR_CHECK((is_same<float, tl_at<l1, 1>::type>::value));
    BOOST_ACTOR_CHECK((is_same<std::string, tl_at<l1, 2>::type>::value));

    BOOST_ACTOR_CHECK_EQUAL(3 , tl_size<l1>::value);
    BOOST_ACTOR_CHECK_EQUAL(tl_size<r1>::value, tl_size<l1>::value);
    BOOST_ACTOR_CHECK((is_same<tl_at<l1, 0>::type, tl_at<r1, 2>::type>::value));
    BOOST_ACTOR_CHECK((is_same<tl_at<l1, 1>::type, tl_at<r1, 1>::type>::value));
    BOOST_ACTOR_CHECK((is_same<tl_at<l1, 2>::type, tl_at<r1, 0>::type>::value));

    typedef tl_concat<type_list<int>, l1>::type l2;

    BOOST_ACTOR_CHECK((is_same<int, tl_head<l2>::type>::value));
    BOOST_ACTOR_CHECK((is_same<l1, tl_tail<l2>::type>::value));

    BOOST_ACTOR_CHECK_EQUAL((detail::tl_count<l1, is_int>::value), 1);
    BOOST_ACTOR_CHECK_EQUAL((detail::tl_count<l2, is_int>::value), 2);

    typedef int_list<0, 1, 2, 3, 4, 5> il0;
    typedef int_list<4, 5> il1;
    typedef typename il_right<il0, 2>::type il2;
    BOOST_ACTOR_CHECK_VERBOSE((is_same<il2, il1>::value),
                       "il_right<il0, 2> returned " <<detail::demangle<il2>()
                       << "expected: " << detail::demangle<il1>());

    /* test tl_is_strict_subset */ {
        typedef type_list<int,float,double> list_a;
        typedef type_list<float,int,double,std::string> list_b;
        BOOST_ACTOR_CHECK((tl_is_strict_subset<list_a, list_b>::value));
        BOOST_ACTOR_CHECK(!(tl_is_strict_subset<list_b, list_a>::value));
        BOOST_ACTOR_CHECK((tl_is_strict_subset<list_a, list_a>::value));
        BOOST_ACTOR_CHECK((tl_is_strict_subset<list_b, list_b>::value));
    }

    return BOOST_ACTOR_TEST_RESULT();
}
