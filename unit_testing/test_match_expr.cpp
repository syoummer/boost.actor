#include "test.hpp"

#include "boost/variant.hpp"

#include "boost/actor/cppa.hpp"
#include "boost/actor/detail/demangle.hpp"

using std::cout;
using std::endl;

using boost::get;
using boost::none;
using boost::none_t;
using boost::variant;

using namespace boost::actor;

#define BOOST_ACTOR_CHECK_VARIANT(Call, Type, Expected)                        \
    {                                                                          \
        auto res = Call ;                                                      \
        if (get< Type >(&res) == nullptr) {                                    \
            BOOST_ACTOR_FAILURE("result has invalid type");                    \
        } else if (*get< Type >(&res) != Expected ) {                          \
            BOOST_ACTOR_FAILURE("expected " << Expected << " found "           \
                                << *get< Type >(&res));                        \
        }                                                                      \
        else {                                                                 \
            BOOST_ACTOR_CHECKPOINT();                                          \
        }                                                                      \
    } static_cast<void>(0)

#define BOOST_ACTOR_CHECK_VARIANT_UNIT(Call)                                   \
    {                                                                          \
        auto res = Call ;                                                      \
        if (get<unit_t>(&res) == nullptr) {                                    \
            BOOST_ACTOR_FAILURE("result has invalid type");                    \
        }                                                                      \
        else {                                                                 \
            BOOST_ACTOR_CHECKPOINT();                                          \
        }                                                                      \
    } static_cast<void>(0)

#define BOOST_ACTOR_CHECK_OPT_MSG(Call, Type, Expected)                        \
    {                                                                          \
        auto res = Call ;                                                      \
        if (!res) {                                                            \
            BOOST_ACTOR_FAILURE("result is none");                             \
        } else if (res->type_at(0) != uniform_typeid< Type >()) {              \
            BOOST_ACTOR_FAILURE(                                               \
                "result has invalid type: expected: "                          \
                << boost::actor::detail::demangle(typeid( Expected ).name())   \
                << ", found: "                                                 \
                << boost::actor::detail::demangle(typeid( Type ).name()));     \
        } else if (res->get_as< Type >(0) != Expected ) {                      \
            BOOST_ACTOR_FAILURE("expected " << Expected << " found "           \
                                << res->get_as< Type >(0));                    \
        }                                                                      \
        else {                                                                 \
            BOOST_ACTOR_CHECKPOINT();                                          \
        }                                                                      \
    } static_cast<void>(0)

#define BOOST_ACTOR_CHECK_OPT_MSG_VOID(Call)                                   \
    {                                                                          \
        auto res = Call ;                                                      \
        if (!res) {                                                            \
            BOOST_ACTOR_FAILURE("result has invalid type: optional is none");  \
        } else if (!res->empty()) {                                            \
            BOOST_ACTOR_FAILURE("result has invalid type: tuple is not empty");\
        } else {                                                               \
            BOOST_ACTOR_CHECKPOINT();                                          \
        }                                                                      \
    } static_cast<void>(0)

#define BOOST_ACTOR_CHECK_OPT_MSG_NONE(Call)                                   \
    {                                                                          \
        auto res = Call ;                                                      \
        if (res) {                                                             \
            if (res->empty()) {                                                \
                BOOST_ACTOR_FAILURE("result has invalid type: expected none, " \
                                    << "found: an empty tuple");               \
            } else {                                                           \
                BOOST_ACTOR_FAILURE(                                           \
                        "result has invalid type: expected none, "             \
                        << "found: "                                           \
                        << detail::demangle(res->type_token()->name()));       \
            }                                                                  \
        }                                                                      \
        else {                                                                 \
            BOOST_ACTOR_CHECKPOINT();                                          \
        }                                                                      \
    } static_cast<void>(0)

struct printer {
    inline void operator()() {
        cout << endl;
    }
    template<typename T, typename... Ts>
    void operator()(const boost::optional<T>& v, const Ts&... vs) {
        if (v) cout << *v << " ";
        else cout << "[none] ";
        (*this)(vs...);
    }
    template<typename T, typename... Ts>
    void operator()(const T& v, const Ts&... vs) {
        cout << v << " ";
        (*this)(vs...);
    }
};

int main() {
    BOOST_ACTOR_TEST(test_match_expr);

    auto g = guarded(std::equal_to<int>{}, 5);
    auto t0 = std::make_tuple(unit, g, unit);
    auto t1 = std::make_tuple(4, 5, 6);
    auto t2 = std::make_tuple(5, 6, 7);
    detail::lifted_fun_zipper zip;
    auto is = detail::get_indices(t2);
    auto r0 = detail::tuple_zip(zip, is, t0, t1);
    cout << "typeid(r0) = " << detail::demangle(typeid(r0).name()) << endl;
    printer p;
    detail::apply_args(p, is, r0);

    { // --- types only ---
        // check on() usage
        auto m0 = on<int>() >> [](int) { };
        //auto m0r0 = m0(make_message(1));
        BOOST_ACTOR_CHECK_VARIANT_UNIT(m0(make_message(1)));
        // check lifted functor
        auto m1 = detail::lift_to_match_expr([](float) { });
        BOOST_ACTOR_CHECK_VARIANT_UNIT(m1(make_message(1.f)));
        // check _.or_else(_)
        auto m2 = m0.or_else(m1);
        BOOST_ACTOR_CHECK_VARIANT_UNIT(m2(make_message(1)));
        BOOST_ACTOR_CHECK_VARIANT_UNIT(m2(make_message(1.f)));
        // check use of match_expr_concat
        auto m3 = detail::match_expr_concat(m0, m1, detail::lift_to_match_expr([](double) { }));
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m3->invoke(make_message(1)));
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m3->invoke(make_message(1.f)));
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m3->invoke(make_message(1.)));
        BOOST_ACTOR_CHECK_OPT_MSG_NONE(m3->invoke(make_message("1")));
    }

    { // --- same with guards ---
        auto m0 = on(1) >> [](int i) { BOOST_ACTOR_CHECK_EQUAL(i, 1); };
        BOOST_ACTOR_CHECK_VARIANT_UNIT(m0(make_message(1)));
        // check lifted functor
        auto m1 = on(1.f) >> [](float) { };
        BOOST_ACTOR_CHECK_VARIANT_UNIT(m1(make_message(1.f)));
        // check _.or_else(_)
        auto m2 = m0.or_else(m1);
        BOOST_ACTOR_CHECK_VARIANT_UNIT(m2(make_message(1)));
        BOOST_ACTOR_CHECK_VARIANT_UNIT(m2(make_message(1.f)));
        // check use of match_expr_concat
        auto m3 = detail::match_expr_concat(m0, m1, on(1.) >> [](double) { });
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m3->invoke(make_message(1)));
        BOOST_ACTOR_CHECK_OPT_MSG_NONE(m3->invoke(make_message(2)));
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m3->invoke(make_message(1.f)));
        BOOST_ACTOR_CHECK_OPT_MSG_NONE(m3->invoke(make_message(2.f)));
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m3->invoke(make_message(1.)));
        BOOST_ACTOR_CHECK_OPT_MSG_NONE(m3->invoke(make_message(2.)));
        BOOST_ACTOR_CHECK_OPT_MSG_NONE(m3->invoke(make_message("1")));
    }

    { // --- mixin it up with partial_function
        // check on() usage
        partial_function m0{on<int>() >> [](int) { }};
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m0(make_message(1)));
        // check lifted functor
        auto m1 = detail::lift_to_match_expr([](float) { });
        BOOST_ACTOR_CHECK_VARIANT_UNIT(m1(make_message(1.f)));
        // check _.or_else(_)
        auto m2 = m0.or_else(m1);
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m2(make_message(1)));
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m2(make_message(1.f)));
        // check use of match_expr_concat
        auto m3 = detail::match_expr_concat(m0, m1, detail::lift_to_match_expr([](double) { }));
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m3->invoke(make_message(1)));
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m3->invoke(make_message(1.)));
        BOOST_ACTOR_CHECK_OPT_MSG_VOID(m3->invoke(make_message(1.f)));
    }

    { // --- use match_expr with result ---
        auto m4 = on<int>() >> [](int i) { return i; };
        BOOST_ACTOR_CHECK_VARIANT(m4(make_message(42)), int, 42);
        auto m5 = on<float>() >> [](float f) { return f; };
        BOOST_ACTOR_CHECK_VARIANT(m5(make_message(4.2f)), float, 4.2f);
        auto m6 = m4.or_else(m5);
        BOOST_ACTOR_CHECK_VARIANT(m6(make_message(4.2f)), float, 4.2f);
        BOOST_ACTOR_CHECK_VARIANT(m6(make_message(42)), int, 42);
    }

    { // --- storing some match_expr in a behavior ---
        behavior m5 {
            on(1  ) >> [] { return 2;   },
            on(1.f) >> [] { return 2.f; },
            on(1. ) >> [] { return 2.;  }
        };
        BOOST_ACTOR_CHECK_OPT_MSG(m5(make_message(1)), int, 2);
        BOOST_ACTOR_CHECK_OPT_MSG(m5(make_message(1.)), double, 2.);
        BOOST_ACTOR_CHECK_OPT_MSG(m5(make_message(1.f)), float, 2.f);
    }

    return BOOST_ACTOR_TEST_RESULT();
}
