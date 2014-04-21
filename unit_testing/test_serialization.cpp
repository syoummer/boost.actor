#include "boost/actor/config.hpp"

#include <new>
#include <set>
#include <list>
#include <stack>
#include <locale>
#include <memory>
#include <string>
#include <limits>
#include <vector>
#include <cstring>
#include <sstream>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <iterator>
#include <typeinfo>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <type_traits>

#include "test.hpp"

#include "boost/actor/cow_tuple.hpp"
#include "boost/actor/any_tuple.hpp"
#include "boost/actor/announce.hpp"
#include "boost/actor/tuple_cast.hpp"
#include "boost/actor/any_tuple.hpp"
#include "boost/actor/to_string.hpp"
#include "boost/actor/serializer.hpp"
#include "boost/actor/from_string.hpp"
#include "boost/actor/ref_counted.hpp"
#include "boost/actor/deserializer.hpp"
#include "boost/actor/primitive_type.hpp"
#include "boost/actor/actor_namespace.hpp"
#include "boost/actor/primitive_variant.hpp"
#include "boost/actor/binary_serializer.hpp"
#include "boost/actor/binary_deserializer.hpp"
#include "boost/actor/util/get_mac_addresses.hpp"

#include "boost/actor/util/pt_token.hpp"
#include "boost/actor/util/int_list.hpp"
#include "boost/actor/util/algorithm.hpp"
#include "boost/actor/util/type_traits.hpp"
#include "boost/actor/util/abstract_uniform_type_info.hpp"

#include "boost/actor/detail/ieee_754.hpp"
#include "boost/actor/detail/object_array.hpp"
#include "boost/actor/detail/type_to_ptype.hpp"
#include "boost/actor/detail/ptype_to_type.hpp"

using namespace std;
using namespace boost::actor;
using namespace boost::actor::util;

using boost::actor::detail::type_to_ptype;
using boost::actor::detail::ptype_to_type;

namespace {

struct struct_a {
    int x;
    int y;
};

struct struct_b {
    struct_a a;
    int z;
    list<int> ints;
};

typedef map<string, u16string> strmap;

struct struct_c {
    strmap strings;
    set<int> ints;
};

struct raw_struct {
    string str;
};

bool operator==(const raw_struct& lhs, const raw_struct& rhs) {
    return lhs.str == rhs.str;
}

struct raw_struct_type_info : util::abstract_uniform_type_info<raw_struct> {
    void serialize(const void* ptr, serializer* sink) const override {
        auto rs = reinterpret_cast<const raw_struct*>(ptr);
        sink->write_value(static_cast<uint32_t>(rs->str.size()));
        sink->write_raw(rs->str.size(), rs->str.data());
    }
    void deserialize(void* ptr, deserializer* source) const override {
        auto rs = reinterpret_cast<raw_struct*>(ptr);
        rs->str.clear();
        auto size = source->read<std::uint32_t>();
        rs->str.resize(size);
        source->read_raw(size, &(rs->str[0]));
    }
    bool equals(const void* lhs, const void* rhs) const override {
        return deref(lhs) == deref(rhs);
    }
};

void test_ieee_754() {
    // check conversion of float
    float f1 = 3.1415925f;                 // float value
    auto p1 = boost::actor::detail::pack754(f1);   // packet value
    BOOST_ACTOR_CHECK_EQUAL(p1, 0x40490FDA);
    auto u1 = boost::actor::detail::unpack754(p1); // unpacked value
    BOOST_ACTOR_CHECK_EQUAL(f1, u1);
    // check conversion of double
    double f2 = 3.14159265358979311600;    // double value
    auto p2 = boost::actor::detail::pack754(f2);   // packet value
    BOOST_ACTOR_CHECK_EQUAL(p2, 0x400921FB54442D18);
    auto u2 = boost::actor::detail::unpack754(p2); // unpacked value
    BOOST_ACTOR_CHECK_EQUAL(f2, u2);
}

enum class test_enum { a, b, c };

} // namespace <anonymous>

int main() {
    BOOST_ACTOR_TEST(test_serialization);

    announce<test_enum>();

    test_ieee_754();

    typedef std::integral_constant<int, detail::impl_id<strmap>()> token;
    BOOST_ACTOR_CHECK_EQUAL(util::is_iterable<strmap>::value, true);
    BOOST_ACTOR_CHECK_EQUAL(detail::is_stl_compliant_list<vector<int>>::value, true);
    BOOST_ACTOR_CHECK_EQUAL(detail::is_stl_compliant_list<strmap>::value, false);
    BOOST_ACTOR_CHECK_EQUAL(detail::is_stl_compliant_map<strmap>::value, true);
    BOOST_ACTOR_CHECK_EQUAL(detail::impl_id<strmap>(), 2);
    BOOST_ACTOR_CHECK_EQUAL(token::value, 2);

    announce(typeid(raw_struct), create_unique<raw_struct_type_info>());

    actor_namespace addressing;

    cout << "process id: " << to_string(get_middleman()->node()) << endl;

    auto oarr = new detail::object_array;
    oarr->push_back(object::from(static_cast<uint32_t>(42)));
    oarr->push_back(object::from("foo"  ));

    any_tuple atuple1{static_cast<any_tuple::raw_ptr>(oarr)};
    try {
        auto opt = tuple_cast<uint32_t, string>(atuple1);
        BOOST_ACTOR_CHECK(opt.valid());
        if (opt) {
            auto& tup = *opt;
            BOOST_ACTOR_CHECK_EQUAL(tup.size(), static_cast<size_t>(2));
            BOOST_ACTOR_CHECK_EQUAL(get<0>(tup), static_cast<uint32_t>(42));
            BOOST_ACTOR_CHECK_EQUAL(get<1>(tup), "foo");
        }
    }
    catch (exception& e) { BOOST_ACTOR_FAILURE(to_verbose_string(e)); }

    detail::meta_cow_tuple<int,int> mct;
    try {
        auto tup0 = make_cow_tuple(1, 2);
        util::buffer wr_buf;
        binary_serializer bs(&wr_buf, &addressing);
        mct.serialize(&tup0, &bs);
        binary_deserializer bd(wr_buf.data(), wr_buf.size(), &addressing);
        auto ptr = mct.new_instance();
        auto ptr_guard = make_scope_guard([&] {
            mct.delete_instance(ptr);
        });
        mct.deserialize(ptr, &bd);
        auto& tref = *reinterpret_cast<cow_tuple<int, int>*>(ptr);
        BOOST_ACTOR_CHECK_EQUAL(get<0>(tup0), get<0>(tref));
        BOOST_ACTOR_CHECK_EQUAL(get<1>(tup0), get<1>(tref));
    }
    catch (exception& e) { BOOST_ACTOR_FAILURE(to_verbose_string(e)); }

    try {
        // test raw_type in both binary and string serialization
        raw_struct rs;
        rs.str = "Lorem ipsum dolor sit amet.";
        util::buffer wr_buf;
        binary_serializer bs(&wr_buf, &addressing);
        bs << rs;
        binary_deserializer bd(wr_buf.data(), wr_buf.size(), &addressing);
        raw_struct rs2;
        uniform_typeid<raw_struct>()->deserialize(&rs2, &bd);
        BOOST_ACTOR_CHECK_EQUAL(rs2.str, rs.str);
        auto rsstr = boost::actor::to_string(object::from(rs));
        rs2.str = "foobar";
        BOOST_ACTOR_PRINT("rs as string: " << rsstr);
        rs2 = from_string<raw_struct>(rsstr);
        BOOST_ACTOR_CHECK_EQUAL(rs2.str, rs.str);
    }
    catch (exception& e) { BOOST_ACTOR_FAILURE(to_verbose_string(e)); }

    try {
        scoped_actor self;
        auto ttup = make_any_tuple(1, 2, actor{self.get()});
        util::buffer wr_buf;
        binary_serializer bs(&wr_buf, &addressing);
        bs << ttup;
        binary_deserializer bd(wr_buf.data(), wr_buf.size(), &addressing);
        any_tuple ttup2;
        uniform_typeid<any_tuple>()->deserialize(&ttup2, &bd);
        BOOST_ACTOR_CHECK(ttup  == ttup2);
    }
    catch (exception& e) { BOOST_ACTOR_FAILURE(to_verbose_string(e)); }

    try {
        scoped_actor self;
        auto ttup = make_any_tuple(1, 2, actor{self.get()});
        util::buffer wr_buf;
        binary_serializer bs(&wr_buf, &addressing);
        bs << ttup;
        bs << ttup;
        binary_deserializer bd(wr_buf.data(), wr_buf.size(), &addressing);
        any_tuple ttup2;
        any_tuple ttup3;
        uniform_typeid<any_tuple>()->deserialize(&ttup2, &bd);
        uniform_typeid<any_tuple>()->deserialize(&ttup3, &bd);
        BOOST_ACTOR_CHECK(ttup  == ttup2);
        BOOST_ACTOR_CHECK(ttup  == ttup3);
        BOOST_ACTOR_CHECK(ttup2 == ttup3);
    }
    catch (exception& e) { BOOST_ACTOR_FAILURE(to_verbose_string(e)); }

    try {
        // serialize b1 to buf
        util::buffer wr_buf;
        binary_serializer bs(&wr_buf, &addressing);
        bs << atuple1;
        // deserialize b2 from buf
        binary_deserializer bd(wr_buf.data(), wr_buf.size(), &addressing);
        any_tuple atuple2;
        uniform_typeid<any_tuple>()->deserialize(&atuple2, &bd);
        auto opt = tuple_cast<uint32_t, string>(atuple2);
        BOOST_ACTOR_CHECK(opt.valid());
        if (opt.valid()) {
            auto& tup = *opt;
            BOOST_ACTOR_CHECK_EQUAL(tup.size(), 2);
            BOOST_ACTOR_CHECK_EQUAL(get<0>(tup), 42);
            BOOST_ACTOR_CHECK_EQUAL(get<1>(tup), "foo");
        }
    }
    catch (exception& e) { BOOST_ACTOR_FAILURE(to_verbose_string(e)); }

    BOOST_ACTOR_CHECK((is_iterable<int>::value) == false);
    // string is primitive and thus not identified by is_iterable
    BOOST_ACTOR_CHECK((is_iterable<string>::value) == false);
    BOOST_ACTOR_CHECK((is_iterable<list<int>>::value) == true);
    BOOST_ACTOR_CHECK((is_iterable<map<int, int>>::value) == true);
    try {  // test meta_object implementation for primitive types
        auto meta_int = uniform_typeid<uint32_t>();
        BOOST_ACTOR_CHECK(meta_int != nullptr);
        if (meta_int) {
            auto o = meta_int->create();
            get_ref<uint32_t>(o) = 42;
            auto str = to_string(object::from(get<uint32_t>(o)));
            BOOST_ACTOR_CHECK_EQUAL( "@u32 ( 42 )", str);
        }
    }
    catch (exception& e) { BOOST_ACTOR_FAILURE(to_verbose_string(e)); }

    // test serialization of enums
    try {
        auto enum_tuple = make_any_tuple(test_enum::b);
        // serialize b1 to buf
        util::buffer wr_buf;
        binary_serializer bs(&wr_buf, &addressing);
        bs << enum_tuple;
        // deserialize b2 from buf
        binary_deserializer bd(wr_buf.data(), wr_buf.size(), &addressing);
        any_tuple enum_tuple2;
        uniform_typeid<any_tuple>()->deserialize(&enum_tuple2, &bd);
        auto opt = tuple_cast<test_enum>(enum_tuple2);
        BOOST_ACTOR_CHECK(opt.valid());
        if (opt.valid()) {
            auto& tup = *opt;
            BOOST_ACTOR_CHECK_EQUAL(tup.size(), 1);
            BOOST_ACTOR_CHECK(get<0>(tup) == test_enum::b);
        }

    }
    catch (exception& e) { BOOST_ACTOR_FAILURE(to_verbose_string(e)); }

    return BOOST_ACTOR_TEST_RESULT();
}
