#include "test.hpp"

#include "boost/variant.hpp"

#include "boost/actor/cppa.hpp"

using boost::get;
using boost::variant;

using namespace boost::actor;

int main() {
    BOOST_ACTOR_TEST(test_match_expr);

    { // --- types only ---
        // check on() usage
        auto m0 = on<int>() >> [](int) { };
        auto m0r0 = m0(make_message(1));
        BOOST_ACTOR_CHECK(get<unit_t>(&m0r0) != nullptr);
        // check lifted functor
        auto m1 = detail::lift_to_match_expr([](float) { });
        auto m1r0 = m1(make_message(1.f));
        BOOST_ACTOR_CHECK(get<unit_t>(&m1r0) != nullptr);
        // check _.or_else(_)
        auto m2 = m0.or_else(m1);
        auto m2r0 = m2(make_message(1));
        auto m2r1 = m2(make_message(1.f));
        BOOST_ACTOR_CHECK(get<unit_t>(&m2r0) != nullptr);
        BOOST_ACTOR_CHECK(get<unit_t>(&m2r1) != nullptr);
        // check use of match_expr_concat
        auto m3 = detail::match_expr_concat(m0, m1, detail::lift_to_match_expr([](double) { }));
        auto m3r0 = m3->invoke(make_message(1));
        auto m3r1 = m3->invoke(make_message(1.f));
        auto m3r2 = m3->invoke(make_message(1.));
        BOOST_ACTOR_CHECK(static_cast<bool>(m3r0));
        BOOST_ACTOR_CHECK(static_cast<bool>(m3r1));
        BOOST_ACTOR_CHECK(static_cast<bool>(m3r2));
    }

    { // --- same with guards ---
        auto m0 = on(1) >> [] { };
        auto m0r0 = m0(make_message(1));
        BOOST_ACTOR_CHECK(get<unit_t>(&m0r0) != nullptr);
        // check lifted functor
        auto m1 = on(1.f) >> [](float) { };
        auto m1r0 = m1(make_message(1.f));
        BOOST_ACTOR_CHECK(get<unit_t>(&m1r0) != nullptr);
        // check _.or_else(_)
        auto m2 = m0.or_else(m1);
        auto m2r0 = m2(make_message(1));
        auto m2r1 = m2(make_message(1.f));
        BOOST_ACTOR_CHECK(get<unit_t>(&m2r0) != nullptr);
        BOOST_ACTOR_CHECK(get<unit_t>(&m2r1) != nullptr);
        // check use of match_expr_concat
        auto m3 = detail::match_expr_concat(m0, m1, on(1.) >> [](double) { });
        auto m3r0 = m3->invoke(make_message(1));
        auto m3r1 = m3->invoke(make_message(1.f));
        auto m3r2 = m3->invoke(make_message(1.));
        BOOST_ACTOR_CHECK(static_cast<bool>(m3r0));
        BOOST_ACTOR_CHECK(static_cast<bool>(m3r1));
        BOOST_ACTOR_CHECK(static_cast<bool>(m3r2));
    }

    { // --- mixin it up with partial_function
        // check on() usage
        partial_function m0{on<int>() >> [](int) { }};
        auto m0r0 = m0(make_message(1));
        BOOST_ACTOR_CHECK(static_cast<bool>(m0r0));
        // check lifted functor
        auto m1 = detail::lift_to_match_expr([](float) { });
        auto m1r0 = m1(make_message(1.f));
        BOOST_ACTOR_CHECK(get<unit_t>(&m1r0) != nullptr);
        // check _.or_else(_)
        auto m2 = m0.or_else(m1);
        auto m2r0 = m2(make_message(1));
        auto m2r1 = m2(make_message(1.f));
        BOOST_ACTOR_CHECK(static_cast<bool>(m2r0));
        BOOST_ACTOR_CHECK(static_cast<bool>(m2r1));
        // check use of match_expr_concat
        auto m3 = detail::match_expr_concat(m0, m1, detail::lift_to_match_expr([](double) { }));
        auto m3r0 = m3->invoke(make_message(1));
        auto m3r1 = m3->invoke(make_message(1.f));
        auto m3r2 = m3->invoke(make_message(1.));
        BOOST_ACTOR_CHECK(static_cast<bool>(m3r0));
        BOOST_ACTOR_CHECK(static_cast<bool>(m3r1));
        BOOST_ACTOR_CHECK(static_cast<bool>(m3r2));
    }

    { // --- storing some match_expr in a behavior ---
        auto m4_ = detail::match_expr_concat(
            on(1  ) >> [] { return 2;   },
            on(1.f) >> [] { return 2.f; },
            on(1. ) >> [] { return 2.;  }
        );
        auto m4r0_ = m4_->invoke(make_message(1));
        BOOST_ACTOR_CHECK(static_cast<bool>(m4r0_));
        behavior m4 = m4_;
        auto msg = make_message(1);
        auto m4r0 = m4(msg);
        if (!m4r0) { BOOST_ACTOR_FAILURE("m4r0 == none"); }
        else if (m4r0->size() != 1) { BOOST_ACTOR_FAILURE("m4r0->size != 1"); }
        else if (m4r0->type_at(0) == uniform_typeid<int>()) { BOOST_ACTOR_FAILURE("result is not an int"); }
        else if (m4r0->get_as<int>(0) == 2) { BOOST_ACTOR_FAILURE("result != 2"); }
    }

    return BOOST_ACTOR_TEST_RESULT();
}
