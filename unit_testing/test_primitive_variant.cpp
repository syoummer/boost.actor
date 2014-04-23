#include "test.hpp"

#include "boost/actor/primitive_variant.hpp"

#include "boost/actor/detail/to_uniform_name.hpp"

namespace {

struct streamer {
    std::ostream& o;
    streamer(std::ostream& mo) : o(mo) { }
    template<typename T>
    void operator()(const T& value) { o << value; }
    void operator()(const std::u16string&) { }
    void operator()(const std::u32string&) { }
};

} // namespace <anonymous>


namespace boost {
namespace actor {
inline std::ostream& operator<<(std::ostream& o, const primitive_variant& pv) {
    apply_visitor(streamer{o}, pv);
    //streamer{o};
    //pv.apply(s);
    return o;
}
} // namespace actor
} // namespace boost

using namespace boost::actor;

int main() {
    BOOST_ACTOR_TEST(test_primitive_variant);

    std::uint32_t forty_two = 42;
    primitive_variant v1(forty_two);
    primitive_variant v2(pt_uint32);
    // type checking
    BOOST_ACTOR_CHECK_EQUAL(v1.ptype(), pt_uint32);
    BOOST_ACTOR_CHECK_EQUAL(v2.ptype(), pt_uint32);
    get_ref<std::uint32_t&>(v2) = forty_two;
    BOOST_ACTOR_CHECK(equal(v1, v2));
    BOOST_ACTOR_CHECK(equal(v1, forty_two));
    BOOST_ACTOR_CHECK(equal(forty_two, v2));
    // type mismatch => unequal
    BOOST_ACTOR_CHECK(!equal(v2, static_cast<std::int8_t>(forty_two)));
    v1 = "Hello world";
    BOOST_ACTOR_CHECK_EQUAL(v1.ptype(), pt_u8string);
    v2 = "Hello";
    BOOST_ACTOR_CHECK_EQUAL(v2.ptype(), pt_u8string);
    get_ref<std::string>(v2) += " world";
    BOOST_ACTOR_CHECK(equal(v1, v2));
    v2 = u"Hello World";
    BOOST_ACTOR_CHECK(!equal(v1, v2));
    primitive_variant v3{atom("hello")};
    BOOST_ACTOR_CHECK(v3.type() == typeid(atom_value));
    BOOST_ACTOR_CHECK(get<atom_value>(v3) == atom("hello"));

    return BOOST_ACTOR_TEST_RESULT();
}
