#include <map>
#include <set>
#include <memory>
#include <cctype>
#include <atomic>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <stdexcept>

#include "test.hpp"

#include "boost/actor/atom.hpp"
#include "boost/actor/announce.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/serializer.hpp"
#include "boost/actor/deserializer.hpp"
#include "boost/actor/message_header.hpp"
#include "boost/actor/uniform_type_info.hpp"

#include "boost/actor/detail/type_traits.hpp"

using std::cout;
using std::endl;

namespace {

struct foo {
    int value;
    explicit foo(int val = 0) : value(val) { }
};

inline bool operator==(const foo& lhs, const foo& rhs) {
    return lhs.value == rhs.value;
}

} // namespace <anonymous>

using namespace boost::actor;

int main() {
    BOOST_ACTOR_TEST(test_uniform_type);
    auto announce1 = announce<foo>(&foo::value);
    auto announce2 = announce<foo>(&foo::value);
    auto announce3 = announce<foo>(&foo::value);
    auto announce4 = announce<foo>(&foo::value);
    BOOST_ACTOR_CHECK(announce1 == announce2);
    BOOST_ACTOR_CHECK(announce1 == announce3);
    BOOST_ACTOR_CHECK(announce1 == announce4);
    BOOST_ACTOR_CHECK_EQUAL(announce1->name(), "$::foo");
    {
        /*
        //bar.create_object();
        auto obj1 = uniform_typeid<foo>()->create();
        auto obj2(obj1);
        BOOST_ACTOR_CHECK(obj1 == obj2);
        get_ref<foo>(obj1).value = 42;
        BOOST_ACTOR_CHECK(obj1 != obj2);
        BOOST_ACTOR_CHECK_EQUAL(get<foo>(obj1).value, 42);
        BOOST_ACTOR_CHECK_EQUAL(get<foo>(obj2).value, 0);
        */
    }
    {
        auto uti = uniform_typeid<atom_value>();
        BOOST_ACTOR_CHECK(uti != nullptr);
        BOOST_ACTOR_CHECK_EQUAL(uti->name(), "@atom");
    }
    // these types (and only those) are present if
    // the uniform_type_info implementation is correct
    std::set<std::string> expected = {
        // local types
        "$::foo",                           // <anonymous namespace>::foo
        // primitive types
        "bool",
        "@i8", "@i16", "@i32", "@i64",      // signed integer names
        "@u8", "@u16", "@u32", "@u64",      // unsigned integer names
        "@str", "@u16str", "@u32str",       // strings
        "float", "double", "@ldouble",      // floating points
        // default announced types
        "@0",                               // unit_t
        "@accept",                          // accept_handle
        "@acceptor_closed",                 // acceptor_closed_msg
        "@actor",                           // actor
        "@addr",                            // actor_addr
        "@atom",                            // atom_value
        "@channel",                         // channel
        "@charbuf",                         // vector<char>
        "@connection",                      // connection_handle
        "@connection_closed",               // connection_closed_msg
        "@down",                            // down_msg
        "@duration",                        // duration
        "@exit",                            // exit_msg
        "@group",                           // group
        "@group_down",                      // group_down_msg
        "@header",                          // message_header
        "@message",                         // message
        "@new_connection",                  // new_connection_msg
        "@new_data",                        // new_data_msg
        "@proc",                            // intrusive_ptr<node_id>
        "@strmap",                          // map<string,string>
        "@sync_exited",                     // sync_exited_msg
        "@sync_timeout",                    // sync_timeout_msg
        "@timeout"                          // timeout_msg
    };
    // holds the type names we see at runtime
    std::set<std::string> found;
    // fetch all available type names
    auto types = uniform_type_info::instances();
    for (auto tinfo : types) {
        found.insert(tinfo->name());
    }
    // compare the two sets
    BOOST_ACTOR_CHECK_EQUAL(expected.size(), found.size());
    bool expected_equals_found = false;
    if (expected.size() == found.size()) {
        expected_equals_found = std::equal(found.begin(),
                                           found.end(),
                                           expected.begin());
        BOOST_ACTOR_CHECK(expected_equals_found);
    }
    if (!expected_equals_found) {
        std::string(41, ' ');
        std::ostringstream oss(std::string(41, ' '));
        oss.seekp(0);
        oss << "found (" << found.size() << ")";
        oss.seekp(22);
        oss << "expected (" << found.size() << ")";
        std::string lhs;
        std::string rhs;
        BOOST_ACTOR_PRINT(oss.str());
        BOOST_ACTOR_PRINT(std::string(41, '-'));
        auto fi = found.begin();
        auto fe = found.end();
        auto ei = expected.begin();
        auto ee = expected.end();
        while (fi != fe || ei != ee) {
            if (fi != fe) lhs = *fi++; else lhs.clear();
            if (ei != ee) rhs = *ei++; else rhs.clear();
            lhs.resize(20, ' ');
            BOOST_ACTOR_PRINT(lhs << "| " << rhs);
        }
    }

    /*
    // check if static types are identical to runtime types
    auto& sarr = detail::static_types_array<
                    int8_t, int16_t, int32_t, int64_t,
                    uint8_t, uint16_t, uint32_t, uint64_t,
                    std::string, std::u16string, std::u32string,
                    float, double,
                    atom_value, message, message_header,
                    actor, group,
                    channel, node_id_ptr
                 >::arr;

    BOOST_ACTOR_CHECK(sarr.is_pure());

    std::vector<const uniform_type_info*> rarr{
        uniform_typeid<int8_t>(),
        uniform_typeid<int16_t>(),
        uniform_typeid<int32_t>(),
        uniform_typeid<int64_t>(),
        uniform_typeid<uint8_t>(),
        uniform_typeid<uint16_t>(),
        uniform_typeid<uint32_t>(),
        uniform_typeid<uint64_t>(),
        uniform_typeid<std::string>(),
        uniform_typeid<std::u16string>(),
        uniform_typeid<std::u32string>(),
        uniform_typeid<float>(),
        uniform_typeid<double>(),
        uniform_typeid<atom_value>(),
        uniform_typeid<message>(),
        uniform_typeid<message_header>(),
        uniform_typeid<actor>(),
        uniform_typeid<group>(),
        uniform_typeid<channel>(),
        uniform_typeid<node_id_ptr>()
    };

    for (size_t i = 0; i < sarr.size; ++i) {
        BOOST_ACTOR_CHECK_EQUAL(sarr[i]->name(), rarr[i]->name());
        BOOST_ACTOR_CHECK(sarr[i] == rarr[i]);
    }

    auto& arr0 = detail::static_types_array<atom_value, uint32_t>::arr;
    BOOST_ACTOR_CHECK(arr0.is_pure());
    BOOST_ACTOR_CHECK(arr0[0] == uniform_typeid<atom_value>());
    BOOST_ACTOR_CHECK(arr0[0] == uniform_type_info::from("@atom"));
    BOOST_ACTOR_CHECK(arr0[1] == uniform_typeid<uint32_t>());
    BOOST_ACTOR_CHECK(arr0[1] == uniform_type_info::from("@u32"));
    BOOST_ACTOR_CHECK(uniform_type_info::from("@u32") == uniform_typeid<uint32_t>());

    auto& arr1 = detail::static_types_array<std::string, int8_t>::arr;
    BOOST_ACTOR_CHECK(arr1[0] == uniform_typeid<std::string>());
    BOOST_ACTOR_CHECK(arr1[0] == uniform_type_info::from("@str"));
    BOOST_ACTOR_CHECK(arr1[1] == uniform_typeid<int8_t>());
    BOOST_ACTOR_CHECK(arr1[1] == uniform_type_info::from("@i8"));

    auto& arr2 = detail::static_types_array<uint8_t, int8_t>::arr;
    BOOST_ACTOR_CHECK(arr2[0] == uniform_typeid<uint8_t>());
    BOOST_ACTOR_CHECK(arr2[0] == uniform_type_info::from("@u8"));
    BOOST_ACTOR_CHECK(arr2[1] == uniform_typeid<int8_t>());
    BOOST_ACTOR_CHECK(arr2[1] == uniform_type_info::from("@i8"));

    auto& arr3 = detail::static_types_array<atom_value, uint16_t>::arr;
    BOOST_ACTOR_CHECK(arr3[0] == uniform_typeid<atom_value>());
    BOOST_ACTOR_CHECK(arr3[0] == uniform_type_info::from("@atom"));
    BOOST_ACTOR_CHECK(arr3[1] == uniform_typeid<uint16_t>());
    BOOST_ACTOR_CHECK(arr3[1] == uniform_type_info::from("@u16"));
    BOOST_ACTOR_CHECK(uniform_type_info::from("@u16") == uniform_typeid<uint16_t>());
    */

    return BOOST_ACTOR_TEST_RESULT();
}
