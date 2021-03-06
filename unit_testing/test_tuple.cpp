#include <list>
#include <array>
#include <string>
#include <limits>
#include <cstdint>
#include <utility>
#include <iostream>
#include <typeinfo>
#include <functional>
#include <type_traits>

#include "test.hpp"

#include "boost/none.hpp"
#include "boost/optional.hpp"

#include "boost/actor/on.hpp"
#include "boost/actor/all.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/to_string.hpp"
#include "boost/intrusive_ptr.hpp"
#include "boost/actor/tmessage_handler.hpp"
#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/detail/type_traits.hpp"

#include "boost/actor/detail/matches.hpp"
#include "boost/actor/detail/projection.hpp"
#include "boost/actor/detail/value_guard.hpp"
#include "boost/actor/detail/object_array.hpp"

using std::cout;
using std::endl;

using boost::none;
using boost::none_t;
using boost::optional;

using namespace boost::actor;
using namespace boost::actor::detail;
using namespace boost::actor::placeholders;


#define BOOST_ACTOR_CHECK_INVOKED(FunName, Args)                               \
    invoked.clear();                                                           \
    {                                                                          \
        auto res = FunName Args ;                                              \
        if (boost::get<none_t>(&res) || invoked != #FunName ) {                \
            BOOST_ACTOR_FAILURE("invocation of " #FunName " failed");          \
        } else { BOOST_ACTOR_CHECKPOINT(); }                                   \
    } static_cast<void>(42)

#define BOOST_ACTOR_CHECK_NOT_INVOKED(FunName, Args)                           \
    invoked.clear();                                                           \
    {                                                                          \
        auto res = FunName Args ;                                              \
        if (boost::get<none_t>(&res) || invoked == #FunName ) {                \
            BOOST_ACTOR_FAILURE(#FunName " erroneously invoked");              \
        } else { BOOST_ACTOR_CHECKPOINT(); }                                   \
    } static_cast<void>(42)

namespace {

std::atomic<size_t> s_expensive_copies;

struct expensive_copy_struct {

    expensive_copy_struct(const expensive_copy_struct& other) : value(other.value) {
        ++s_expensive_copies;
    }

    expensive_copy_struct(expensive_copy_struct&& other) : value(other.value) { }

    expensive_copy_struct() : value(0) { }

    int value;

};

inline bool operator==(const expensive_copy_struct& lhs,
                       const expensive_copy_struct& rhs) {
    return lhs.value == rhs.value;
}

std::string int2str(int i) {
    return std::to_string(i);
}

optional<int> str2int(const std::string& str) {
    char* endptr = nullptr;
    int result = static_cast<int>(strtol(str.c_str(), &endptr, 10));
    if (endptr != nullptr && *endptr == '\0') {
        return result;
    }
    return none;
}

struct dummy_receiver : event_based_actor {
    behavior make_behavior() override {
        return (
            on_arg_match >> [=](expensive_copy_struct& ecs) -> expensive_copy_struct {
                ecs.value = 42;
                quit();
                return std::move(ecs);
            }
        );
    }
};

template<typename First, typename Second>
struct same_second_type : std::is_same<typename First::second, typename Second::second> { };

void check_type_list() {
    using namespace boost::actor::util;

    typedef type_list<int, int, int, float, int, float, float> zz0;

    typedef type_list<type_list<int, int, int>,
                      type_list<float>,
                      type_list<int>,
                      type_list<float, float>> zz8;

    typedef type_list<
                type_list<
                    type_pair<std::integral_constant<size_t, 0>, int>,
                    type_pair<std::integral_constant<size_t, 1>, int>,
                    type_pair<std::integral_constant<size_t, 2>, int>
                >,
                type_list<
                    type_pair<std::integral_constant<size_t, 3>, float>
                >,
                type_list<
                    type_pair<std::integral_constant<size_t, 4>, int>
                >,
                type_list<
                    type_pair<std::integral_constant<size_t, 5>, float>,
                    type_pair<std::integral_constant<size_t, 6>, float>
                >
            >
            zz9;

    typedef typename tl_group_by<zz0, std::is_same>::type zz1;

    typedef typename tl_zip_with_index<zz0>::type zz2;

    static_assert(std::is_same<zz1, zz8>::value, "tl_group_by failed");

    typedef typename tl_group_by<zz2, same_second_type>::type zz3;

    static_assert(std::is_same<zz3, zz9>::value, "tl_group_by failed");
}

void check_guards() {
    BOOST_ACTOR_PRINT(__func__);

    std::string invoked;

    auto f00 = on<int, int>() >> [&]() { invoked = "f00"; };
    BOOST_ACTOR_CHECK_INVOKED(f00, (42, 42));

    auto f01 = on<int, int>().when(_x1 == 42) >> [&]() { invoked = "f01"; };
    BOOST_ACTOR_CHECK_INVOKED(f01, (42, 42));
    BOOST_ACTOR_CHECK_NOT_INVOKED(f01, (1, 2));

    auto f02 = on<int, int>().when(_x1 == 42 && _x2 * 2 == _x1) >> [&]() { invoked = "f02"; };
    BOOST_ACTOR_CHECK_NOT_INVOKED(f02, (0, 0));
    BOOST_ACTOR_CHECK_NOT_INVOKED(f02, (42, 42));
    BOOST_ACTOR_CHECK_NOT_INVOKED(f02, (2, 1));
    BOOST_ACTOR_CHECK_INVOKED(f02, (42, 21));

    BOOST_ACTOR_CHECK(f02(make_message(42, 21)));
    BOOST_ACTOR_CHECK_EQUAL(invoked, "f02");
    invoked.clear();

    auto f03 = on(42, val<int>) >> [&](const int& a, int&) { invoked = "f03"; BOOST_ACTOR_CHECK_EQUAL(a, 42); };
    BOOST_ACTOR_CHECK_NOT_INVOKED(f03, (0, 0));
    BOOST_ACTOR_CHECK_INVOKED(f03, (42, 42));

    auto f04 = on(42, int2str).when(_x2 == "42") >> [&](std::string& str) {
        BOOST_ACTOR_CHECK_EQUAL(str, "42");
        invoked = "f04";
    };

    BOOST_ACTOR_CHECK_NOT_INVOKED(f04, (0, 0));
    BOOST_ACTOR_CHECK_NOT_INVOKED(f04, (0, 42));
    BOOST_ACTOR_CHECK_NOT_INVOKED(f04, (42, 0));
    BOOST_ACTOR_CHECK_INVOKED(f04, (42, 42));

    auto f05 = on(str2int).when(_x1 % 2 == 0) >> [&]() { invoked = "f05"; };
    BOOST_ACTOR_CHECK_NOT_INVOKED(f05, ("1"));
    BOOST_ACTOR_CHECK_INVOKED(f05, ("2"));

    auto f06 = on(42, str2int).when(_x2 % 2 == 0) >> [&]() { invoked = "f06"; };
    BOOST_ACTOR_CHECK_NOT_INVOKED(f06, (0, "0"));
    BOOST_ACTOR_CHECK_NOT_INVOKED(f06, (42, "1"));
    BOOST_ACTOR_CHECK_INVOKED(f06, (42, "2"));

    int f07_val = 1;
    auto f07 = on<int>().when(_x1 == gref(f07_val)) >> [&]() { invoked = "f07"; };
    BOOST_ACTOR_CHECK_NOT_INVOKED(f07, (0));
    BOOST_ACTOR_CHECK_INVOKED(f07, (1));
    BOOST_ACTOR_CHECK_NOT_INVOKED(f07, (2));
    ++f07_val;
    BOOST_ACTOR_CHECK_NOT_INVOKED(f07, (0));
    BOOST_ACTOR_CHECK_NOT_INVOKED(f07, (1));
    BOOST_ACTOR_CHECK_INVOKED(f07, (2));
    BOOST_ACTOR_CHECK(f07(make_message(2)));

    int f08_val = 666;
    auto f08 = on<int>() >> [&](int& mref) { mref = 8; invoked = "f08"; };
    BOOST_ACTOR_CHECK_INVOKED(f08, (f08_val));
    BOOST_ACTOR_CHECK_EQUAL(f08_val, 8);
    message f08_any_val = make_message(666);
    BOOST_ACTOR_CHECK(f08(f08_any_val));
    BOOST_ACTOR_CHECK_EQUAL(f08_any_val.get_as<int>(0), 8);

    int f09_val = 666;
    auto f09 = on(str2int, val<int>) >> [&](int& mref) { mref = 9; invoked = "f09"; };
    BOOST_ACTOR_CHECK_NOT_INVOKED(f09, ("hello lambda", f09_val));
    BOOST_ACTOR_CHECK_INVOKED(f09, ("0", f09_val));
    BOOST_ACTOR_CHECK_EQUAL(f09_val, 9);
    message f09_any_val = make_message("0", 666);
    BOOST_ACTOR_CHECK(f09(f09_any_val));
    BOOST_ACTOR_CHECK_EQUAL(f09_any_val.get_as<int>(1), 9);
    f09_any_val.get_as_mutable<int>(1) = 666;
    message f09_any_val_copy{f09_any_val};
    BOOST_ACTOR_CHECK(f09_any_val.at(0) == f09_any_val_copy.at(0));
    // detaches f09_any_val from f09_any_val_copy
    BOOST_ACTOR_CHECK(f09(f09_any_val));
    BOOST_ACTOR_CHECK_EQUAL(f09_any_val.get_as<int>(1), 9);
    BOOST_ACTOR_CHECK_EQUAL(f09_any_val_copy.get_as<int>(1), 666);
    // no longer the same data
    BOOST_ACTOR_CHECK(f09_any_val.at(0) != f09_any_val_copy.at(0));

    auto f10 = (
        on<int>().when(_x1 < 10)    >> [&]() { invoked = "f10.0"; },
        on<int>()                   >> [&]() { invoked = "f10.1"; },
        on<std::string, anything>() >> [&](std::string&) { invoked = "f10.2"; }
    );

    BOOST_ACTOR_CHECK(f10(9));
    BOOST_ACTOR_CHECK_EQUAL(invoked, "f10.0");
    BOOST_ACTOR_CHECK(f10(10));
    BOOST_ACTOR_CHECK_EQUAL(invoked, "f10.1");
    BOOST_ACTOR_CHECK(f10("42"));
    BOOST_ACTOR_CHECK_EQUAL(invoked, "f10.2");
    BOOST_ACTOR_CHECK(f10("42", 42));
    BOOST_ACTOR_CHECK(f10("a", "b", "c"));
    std::string foobar = "foobar";
    BOOST_ACTOR_CHECK(f10(foobar, "b", "c"));
    BOOST_ACTOR_CHECK(f10("a", static_cast<const std::string&>(foobar), "b", "c"));
}

void check_many_cases() {
    BOOST_ACTOR_PRINT(__func__);
    auto on_int = on<int>();

    int f11_fun = 0;
    auto f11 = (
        on_int.when(_x1 == 1) >> [&] { f11_fun =  1; },
        on_int.when(_x1 == 2) >> [&] { f11_fun =  2; },
        on_int.when(_x1 == 3) >> [&] { f11_fun =  3; },
        on_int.when(_x1 == 4) >> [&] { f11_fun =  4; },
        on_int.when(_x1 == 5) >> [&] { f11_fun =  5; },
        on_int.when(_x1 == 6) >> [&] { f11_fun =  6; },
        on_int.when(_x1 == 7) >> [&] { f11_fun =  7; },
        on_int.when(_x1 == 8) >> [&] { f11_fun =  8; },
        on_int.when(_x1 >= 9) >> [&] { f11_fun =  9; },
        on(str2int)           >> [&] { f11_fun = 10; },
        on<std::string>()     >> [&] { f11_fun = 11; }
    );

    BOOST_ACTOR_CHECK(f11(1));
    BOOST_ACTOR_CHECK_EQUAL(f11_fun, 1);
    BOOST_ACTOR_CHECK(f11(3));
    BOOST_ACTOR_CHECK_EQUAL(f11_fun, 3);
    BOOST_ACTOR_CHECK(f11(8));
    BOOST_ACTOR_CHECK_EQUAL(f11_fun, 8);
    BOOST_ACTOR_CHECK(f11(10));
    BOOST_ACTOR_CHECK_EQUAL(f11_fun, 9);
    BOOST_ACTOR_CHECK(f11("hello lambda"));
    BOOST_ACTOR_CHECK_EQUAL(f11_fun, 11);
    BOOST_ACTOR_CHECK(f11("10"));
    BOOST_ACTOR_CHECK_EQUAL(f11_fun, 10);
}

void check_wildcards() {
    BOOST_ACTOR_PRINT(__func__);
    std::string invoked;

    auto f12 = (
        on<int, anything, int>().when(_x1 < _x2) >> [&](int a, int b) {
            BOOST_ACTOR_CHECK_EQUAL(a, 1);
            BOOST_ACTOR_CHECK_EQUAL(b, 5);
            invoked = "f12";
        }
    );
    BOOST_ACTOR_CHECK_INVOKED(f12, (1, 2, 3, 4, 5));

    int f13_fun = 0;
    auto f13 = (
        on<int, anything, std::string, anything, int>().when(_x1 < _x3 && _x2.starts_with("-")) >> [&](int a, const std::string& str, int b) {
            BOOST_ACTOR_CHECK_EQUAL(str, "-h");
            BOOST_ACTOR_CHECK_EQUAL(a, 1);
            BOOST_ACTOR_CHECK_EQUAL(b, 10);
            f13_fun = 1;
            invoked = "f13";
        },
        on<anything, std::string, anything, int, anything, float, anything>() >> [&](const std::string& str, int a, float b) {
            BOOST_ACTOR_CHECK_EQUAL(str, "h");
            BOOST_ACTOR_CHECK_EQUAL(a, 12);
            BOOST_ACTOR_CHECK_EQUAL(b, 1.f);
            f13_fun = 2;
            invoked = "f13";
        },
        on<float, anything, float>().when(_x1 * 2 == _x2) >> [&](float a, float b) {
            BOOST_ACTOR_CHECK_EQUAL(a, 1.f);
            BOOST_ACTOR_CHECK_EQUAL(b, 2.f);
            f13_fun = 3;
            invoked = "f13";
        }
    );
    BOOST_ACTOR_CHECK_INVOKED(f13, (1, 2, "-h", 12, 32, 10, 1.f, "--foo", 10));
    BOOST_ACTOR_CHECK_EQUAL(f13_fun, 1);
    BOOST_ACTOR_CHECK_INVOKED(f13, (1, 2, "h", 12, 32, 10, 1.f, "--foo", 10));
    BOOST_ACTOR_CHECK_EQUAL(f13_fun, 2);
    BOOST_ACTOR_CHECK_INVOKED(f13, (1.f, 1.5f, 2.f));
    BOOST_ACTOR_CHECK_EQUAL(f13_fun, 3);

    // check type correctness of make_message()
    auto t0 = make_message("1", 2);
    auto t0_0 = get<0>(t0);
    auto t0_1 = get<1>(t0);
    // check implicit type conversion
    BOOST_ACTOR_CHECK((std::is_same<decltype(t0_0), std::string>::value));
    BOOST_ACTOR_CHECK((std::is_same<decltype(t0_1), int>::value));
    BOOST_ACTOR_CHECK_EQUAL("1", t0_0);
    BOOST_ACTOR_CHECK_EQUAL(2, t0_1);
    // use tuple cast to get a subtuple
    message at0(t0);
    auto v0opt = tuple_cast<std::string, anything>(at0);
    BOOST_ACTOR_CHECK((v0opt));
    BOOST_ACTOR_CHECK(   at0.size() == 2
               && at0.at(0) == &get<0>(t0)
               && at0.at(1) == &get<1>(t0));
    if (v0opt) {
        auto& v0 = *v0opt;
        BOOST_ACTOR_CHECK((std::is_same<decltype(get<0>(v0)), const std::string&>::value));
        BOOST_ACTOR_CHECK_EQUAL(v0.size(), 1);
        BOOST_ACTOR_CHECK_EQUAL(get<0>(v0), "1");
        BOOST_ACTOR_CHECK_EQUAL(get<0>(t0), get<0>(v0));
        // check cow semantics
        BOOST_ACTOR_CHECK(&get<0>(t0) == &get<0>(v0));         // point to the same
        get_ref<0>(t0) = "hello world";                 // detaches t0 from v0
        BOOST_ACTOR_CHECK_EQUAL(get<0>(t0), "hello world");    // t0 contains new value
        BOOST_ACTOR_CHECK_EQUAL(get<0>(v0), "1");              // v0 contains old value
        BOOST_ACTOR_CHECK(&get<0>(t0) != &get<0>(v0));         // no longer the same
        // check operator==
        auto lhs = make_message(1, 2, 3, 4);
        auto rhs = make_message(static_cast<std::uint8_t>(1), 2.0, 3, 4);
        BOOST_ACTOR_CHECK(lhs == rhs);
        BOOST_ACTOR_CHECK(rhs == lhs);
    }
    message at1 = make_message("one", 2, 3.f, 4.0); {
        // perfect match
        auto opt0 = tuple_cast<std::string, int, float, double>(at1);
        BOOST_ACTOR_CHECK(opt0);
        if (opt0) {
            BOOST_ACTOR_CHECK((*opt0 == make_message("one", 2, 3.f, 4.0)));
            BOOST_ACTOR_CHECK(&get<0>(*opt0) == at1.at(0));
            BOOST_ACTOR_CHECK(&get<1>(*opt0) == at1.at(1));
            BOOST_ACTOR_CHECK(&get<2>(*opt0) == at1.at(2));
            BOOST_ACTOR_CHECK(&get<3>(*opt0) == at1.at(3));
        }
        // leading wildcard
        auto opt1 = tuple_cast<anything, double>(at1);
        BOOST_ACTOR_CHECK(opt1);
        if (opt1) {
            BOOST_ACTOR_CHECK_EQUAL(get<0>(*opt1), 4.0);
            BOOST_ACTOR_CHECK(&get<0>(*opt1) == at1.at(3));
        }
        // trailing wildcard
        auto opt2 = tuple_cast<std::string, anything>(at1);
        BOOST_ACTOR_CHECK(opt2);
        if (opt2) {
            BOOST_ACTOR_CHECK_EQUAL(get<0>(*opt2), "one");
            BOOST_ACTOR_CHECK(&get<0>(*opt2) == at1.at(0));
        }
        // wildcard in between
        auto opt3 = tuple_cast<std::string, anything, double>(at1);
        BOOST_ACTOR_CHECK(opt3);
        if (opt3) {
            BOOST_ACTOR_CHECK((*opt3 == make_message("one", 4.0)));
            BOOST_ACTOR_CHECK_EQUAL(get<0>(*opt3), "one");
            BOOST_ACTOR_CHECK_EQUAL(get<1>(*opt3), 4.0);
            BOOST_ACTOR_CHECK(&get<0>(*opt3) == at1.at(0));
            BOOST_ACTOR_CHECK(&get<1>(*opt3) == at1.at(3));
        }
        auto opt4 = tuple_cast<anything, double>(at1);
        BOOST_ACTOR_CHECK(opt4);
        if (opt4) {
            BOOST_ACTOR_CHECK((*opt4 == make_message(4.0)));
            BOOST_ACTOR_CHECK_EQUAL(get<0>(*opt4), 4.0);
            BOOST_ACTOR_CHECK(&get<0>(*opt4) == at1.at(3));
        }
    }
}

void check_move_optional() {
    BOOST_ACTOR_PRINT(__func__);
    optional<expensive_copy_struct> opt{expensive_copy_struct{}};
    opt->value = 23;
    auto opt2 = std::move(opt);
    auto move_fun = [](expensive_copy_struct& value) -> optional<expensive_copy_struct> {
        return std::move(value);
    };
    auto opt3 = move_fun(*opt2);
    BOOST_ACTOR_CHECK(opt3.valid());
    BOOST_ACTOR_CHECK_EQUAL(opt->value, 23);
    BOOST_ACTOR_CHECK_EQUAL(s_expensive_copies.load(), 0);
}

void check_move_ops() {
    check_move_optional();
    BOOST_ACTOR_PRINT(__func__);
    BOOST_ACTOR_CHECK_EQUAL(s_expensive_copies.load(), 0);
    scoped_actor self;
    self->send(spawn<dummy_receiver>(), expensive_copy_struct());
    self->receive (
        on_arg_match >> [&](expensive_copy_struct& ecs) {
            BOOST_ACTOR_CHECK_EQUAL(42, ecs.value);
        }
    );
    BOOST_ACTOR_CHECK_EQUAL(s_expensive_copies.load(), 0);
}

void check_drop() {
    BOOST_ACTOR_PRINT(__func__);
    auto t0 = make_message(0, 1, 2, 3);
    auto t1 = t0.drop(2);
    BOOST_ACTOR_CHECK_EQUAL(t1.size(), 2);
    BOOST_ACTOR_CHECK_EQUAL(t1.get_as<int>(0), 2);
    BOOST_ACTOR_CHECK_EQUAL(t1.get_as<int>(1), 3);
    BOOST_ACTOR_CHECK(t1 == make_message(2, 3));
    auto t2 = t0.drop_right(2);
    BOOST_ACTOR_CHECK_EQUAL(t2.size(), 2);
    BOOST_ACTOR_CHECK_EQUAL(t2.get_as<int>(0), 0);
    BOOST_ACTOR_CHECK_EQUAL(t2.get_as<int>(1), 1);
    BOOST_ACTOR_CHECK(t2 == make_message(0, 1));
    BOOST_ACTOR_CHECK(t0.take(3) == t0.drop_right(1));
    BOOST_ACTOR_CHECK(t0.take_right(3) == t0.drop(1));
    BOOST_ACTOR_CHECK(t0 == t0.take(4));
    BOOST_ACTOR_CHECK(t0.take(4) == t0.take_right(20));
    BOOST_ACTOR_CHECK(t0.take(0).empty());
}

} // namespace <anonymous>

int main() {
    BOOST_ACTOR_TEST(test_tuple);
    announce<expensive_copy_struct>(&expensive_copy_struct::value);
    check_type_list();
    check_default_ctors();
    check_guards();
    check_many_cases();
    check_wildcards();
    check_move_ops();
    check_drop();
    await_all_actors_done();
    shutdown();
    return BOOST_ACTOR_TEST_RESULT();
}
