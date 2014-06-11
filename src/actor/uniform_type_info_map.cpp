/******************************************************************************\
 *                                                                            *
 *           ____                  _        _        _                        *
 *          | __ )  ___   ___  ___| |_     / \   ___| |_ ___  _ __            *
 *          |  _ \ / _ \ / _ \/ __| __|   / _ \ / __| __/ _ \| '__|           *
 *          | |_) | (_) | (_) \__ \ |_ _ / ___ \ (__| || (_) | |              *
 *          |____/ \___/ \___/|___/\__(_)_/   \_\___|\__\___/|_|              *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the Boost Software License, Version 1.0. See             *
 * accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt  *
\******************************************************************************/


#include <ios>         // std::ios_base::failure
#include <array>
#include <limits>
#include <string>
#include <vector>
#include <cstring>     // memcmp
#include <algorithm>
#include <type_traits>

#include "boost/thread/lock_types.hpp"

#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#include "boost/actor/group.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/announce.hpp"
#include "boost/actor/duration.hpp"
#include "boost/actor/actor_cast.hpp"
#include "boost/actor/abstract_group.hpp"
#include "boost/actor/actor_namespace.hpp"
#include "boost/actor/message_builder.hpp"

#include "boost/actor/detail/logging.hpp"
#include "boost/actor/detail/safe_equal.hpp"
#include "boost/actor/detail/singletons.hpp"
#include "boost/actor/detail/scope_guard.hpp"
#include "boost/actor/detail/shared_spinlock.hpp"
#include "boost/actor/detail/uniform_type_info_map.hpp"
#include "boost/actor/detail/default_uniform_type_info.hpp"

namespace boost {
namespace actor {
namespace detail {

// maps demangled names to uniform names
// WARNING: this map is sorted, insert new elements *in sorted order* as well!
/* extern */ const char* mapped_type_names[][2] = {
    { "bool",                                           "bool"                },
    { "boost::actor::accept_handle",                    "@accept"             },
    { "boost::actor::acceptor_closed_msg",              "@acceptor_closed"    },
    { "boost::actor::actor",                            "@actor"              },
    { "boost::actor::actor_addr",                       "@addr"               },
    { "boost::actor::atom_value",                       "@atom"               },
    { "boost::actor::channel",                          "@channel"            },
    { "boost::actor::connection_closed_msg",            "@connection_closed"  },
    { "boost::actor::connection_handle",                "@connection"         },
    { "boost::actor::down_msg",                         "@down"               },
    { "boost::actor::duration",                         "@duration"           },
    { "boost::actor::exit_msg",                         "@exit"               },
    { "boost::actor::group",                            "@group"              },
    { "boost::actor::group_down_msg",                   "@group_down"         },
    { "boost::actor::message",                          "@message"            },
    { "boost::actor::message_id",                       "@message_id"         },
    { "boost::actor::new_connection_msg",               "@new_connection"     },
    { "boost::actor::new_data_msg",                     "@new_data"           },
    { "boost::actor::sync_exited_msg",                  "@sync_exited"        },
    { "boost::actor::sync_timeout_msg",                 "@sync_timeout"       },
    { "boost::actor::timeout_msg",                      "@timeout"            },
    { "boost::actor::unit_t",                           "@0"                  },
    { "boost::intrusive_ptr<boost::actor::node_id>",    "@proc"               },
    { "double",                                         "double"              },
    { "float",                                          "float"               },
    { "long double",                                    "@ldouble"            },
    // std::u16string
    { "std::basic_string<@i16,std::char_traits<@i16>,std::allocator<@i16>>",
      "@u16str"                                                               },
    // std::u32string
    { "std::basic_string<@i32,std::char_traits<@i32>,std::allocator<@i32>>",
      "@u32str"                                                               },
    // std::string
    { "std::basic_string<@i8,std::char_traits<@i8>,std::allocator<@i8>>",
      "@str"                                                                  },
    // std::u16string (again, using unsigned char type)
    { "std::basic_string<@u16,std::char_traits<@u16>,std::allocator<@u16>>",
      "@u16str"                                                               },
    // std::u32string (again, using unsigned char type)
    { "std::basic_string<@u32,std::char_traits<@u32>,std::allocator<@u32>>",
      "@u32str"                                                               },
    // std::map<std::string,std::string>
    { "std::map<@str,@str,std::less<@str>,"
               "std::allocator<std::pair<const @str,@str>>>",
      "@strmap"                                                               },
    { "std::vector<@i8,std::allocator<@i8>>",           "@charbuf"            }
};

// maps sizeof(T) => {unsigned name, signed name}
/* extern */ const char* mapped_int_names[][2] = {
    {nullptr, nullptr}, // no int type with 0 bytes
    {"@u8",  "@i8"},
    {"@u16", "@i16"},
    {nullptr, nullptr}, // no int type with 3 bytes
    {"@u32", "@i32"},
    {nullptr, nullptr}, // no int type with 5 bytes
    {nullptr, nullptr}, // no int type with 6 bytes
    {nullptr, nullptr}, // no int type with 7 bytes
    {"@u64", "@i64"}
};

const char* mapped_name_by_decorated_name(const char* cstr) {
    using namespace std;
    auto cmp = [](const char** lhs, const char* rhs) {
        return strcmp(lhs[0], rhs) < 0;
    };
    auto e = end(mapped_type_names);
    auto i = lower_bound(begin(mapped_type_names), e, cstr, cmp);
    if (i != e && strcmp(cstr, (*i)[0]) == 0) return (*i)[1];
    // for some reason, GCC returns "std::string" as RTTI type name
    // instead of std::basic_string<...>, this also affects clang-compiled
    // code when used with GCC's libstdc++
    if (strcmp("std::string", cstr) == 0) return mapped_name<std::string>();
    return cstr;
}

std::string mapped_name_by_decorated_name(std::string&& str) {
    auto res = mapped_name_by_decorated_name(str.c_str());
    if (res == str.c_str()) return std::move(str);
    return res;
}

namespace {

inline bool operator==(const unit_t&, const unit_t&) {
    return true;
}

template<typename T> struct type_token { };

template<typename T>
inline typename std::enable_if<detail::is_primitive<T>::value>::type
serialize_impl(const T& val, serializer* sink) {
    sink->write_value(val);
}

template<typename T>
inline typename std::enable_if<detail::is_primitive<T>::value>::type
deserialize_impl(T& val, deserializer* source) {
    val = source->read<T>();
}

inline void serialize_impl(const unit_t&, serializer*) { }

inline void deserialize_impl(unit_t&, deserializer*) { }

void serialize_impl(const actor_addr& addr, serializer* sink) {
    auto ns = sink->get_namespace();
    if (ns) ns->write(sink, addr);
    else throw std::runtime_error("unable to serialize actor_addr: "
                                  "no actor addressing defined");
}

void deserialize_impl(actor_addr& addr, deserializer* source) {
    auto ns = source->get_namespace();
    if (ns) addr = ns->read(source);
    else throw std::runtime_error("unable to deserialize actor_ptr: "
                                  "no actor addressing defined");
}

void serialize_impl(const actor& ptr, serializer* sink) {
    serialize_impl(ptr.address(), sink);
}

void deserialize_impl(actor& ptr, deserializer* source) {
    actor_addr addr;
    deserialize_impl(addr, source);
    ptr = actor_cast<actor>(addr);
}

void serialize_impl(const group& gref, serializer* sink) {
    if (!gref) {
        BOOST_ACTOR_LOGF_DEBUG("serialized an invalid group");
        // write an empty string as module name
        std::string empty_string;
        sink->write_value(empty_string);
    }
    else {
        sink->write_value(gref->module_name());
        gref->serialize(sink);
    }
}

void deserialize_impl(group& gref, deserializer* source) {
    auto modname = source->read<std::string>();
    if (modname.empty()) gref = invalid_group;
    else gref = group::get_module(modname)->deserialize(source);
}

void serialize_impl(const channel& chref, serializer* sink) {
    // channel is an abstract base class that's either an actor or a group
    // to indicate that, we write a flag first, that is
    //     0 if ptr == nullptr
    //     1 if ptr points to an actor
    //     2 if ptr points to a group
    uint8_t flag = 0;
    auto wr_nullptr = [&] {
        sink->write_value(flag);
    };
    if (!chref) {
        // invalid channel
        wr_nullptr();
    }
    else {
        // raw pointer
        auto rptr = actor_cast<abstract_channel*>(chref);
        // raw actor pointer
        abstract_actor_ptr aptr = dynamic_cast<abstract_actor*>(rptr);
        if (aptr != nullptr) {
            flag = 1;
            sink->write_value(flag);
            serialize_impl(actor_cast<actor>(aptr), sink);
        }
        else {
            // get raw group pointer and store it inside a group handle
            group tmp{dynamic_cast<abstract_group*>(rptr)};
            if (tmp) {
                flag = 2;
                sink->write_value(flag);
                serialize_impl(tmp, sink);
            }
            else {
                BOOST_ACTOR_LOGF_ERROR("ptr is neither an actor nor a group");
                wr_nullptr();
            }
        }
    }
}

void deserialize_impl(channel& ptrref, deserializer* source) {
    auto flag = source->read<uint8_t>();
    switch (flag) {
        case 0: {
            ptrref = channel{}; //ptrref.reset();
            break;
        }
        case 1: {
            actor tmp;
            deserialize_impl(tmp, source);
            ptrref = actor_cast<channel>(tmp);
            break;
        }
        case 2: {
            group tmp;
            deserialize_impl(tmp, source);
            ptrref = tmp;
            break;
        }
        default: {
            BOOST_ACTOR_LOGF_ERROR("invalid flag while deserializing 'channel'");
            throw std::runtime_error("invalid flag");
        }
    }
}

void serialize_impl(const message& tup, serializer* sink) {
    std::string dynamic_name; // used if tup holds an object_array
    // ttn can be nullptr even if tuple is not empty (in case of object_array)
    const std::string* ttn = tup.empty() ? nullptr : tup.tuple_type_names();
    const char* tname = ttn ? ttn->data() : (tup.empty() ? "@<>" : nullptr);
    if (!tname) {
        // tuple is not empty, i.e., we are dealing with an object array
        dynamic_name = detail::get_tuple_type_names(*tup.vals());
        tname = dynamic_name.c_str();
    }
    auto uti = detail::singletons::get_uniform_type_info_map()
               ->by_uniform_name(tname);
    if (uti == nullptr) {
        std::string err = "could not get uniform type info for \"";
        err += tname;
        err += "\"";
        BOOST_ACTOR_LOGF_ERROR(err);
        throw std::runtime_error(err);
    }
    sink->begin_object(uti);
    for (size_t i = 0; i < tup.size(); ++i) {
        tup.type_at(i)->serialize(tup.at(i), sink);
    }
    sink->end_object();
}

void deserialize_impl(message& atref, deserializer* source) {
    auto uti = source->begin_object();
    auto uval = uti->create();
    //auto ptr = uti->new_instance();
    uti->deserialize(uval->val, source);
    source->end_object();
    atref = uti->as_message(uval->val);
}

void serialize_impl(const node_id_ptr& ptr, serializer* sink) {
    if (ptr == nullptr) {
        node_id::serialize_invalid(sink);
    }
    else {
        sink->write_value(ptr->process_id());
        sink->write_raw(ptr->host_id().size(), ptr->host_id().data());
    }
}

void deserialize_impl(node_id_ptr& ptr, deserializer* source) {
    node_id::host_id_type nid;
    auto pid = source->read<uint32_t>();
    source->read_raw(node_id::host_id_size, nid.data());
    auto is_zero = [](uint8_t value) { return value == 0; };
    if (pid == 0 && std::all_of(nid.begin(), nid.end(), is_zero)) {
        // invalid process information (nullptr)
        ptr.reset();
    }
    else ptr.reset(new node_id{pid, nid});
}

inline void serialize_impl(const atom_value& val, serializer* sink) {
    sink->write_value(val);
}

inline void deserialize_impl(atom_value& val, deserializer* source) {
    val = source->read<atom_value>();
}

inline void serialize_impl(const duration& val, serializer* sink) {
    sink->write_value(static_cast<uint32_t>(val.unit));
    sink->write_value(val.count);
}

inline void deserialize_impl(duration& val, deserializer* source) {
    auto unit_val = source->read<uint32_t>();
    auto count_val = source->read<uint32_t>();
    switch (unit_val) {
        case 1:
            val.unit = time_unit::seconds;
            break;

        case 1000:
            val.unit = time_unit::milliseconds;
            break;

        case 1000000:
            val.unit = time_unit::microseconds;
            break;

        default:
            val.unit = time_unit::invalid;
            break;
    }
    val.count = count_val;
}

inline void serialize_impl(bool val, serializer* sink) {
    sink->write_value(static_cast<uint8_t>(val ? 1 : 0));
}

inline void deserialize_impl(bool& val, deserializer* source) {
    val = source->read<uint8_t>() != 0;
}

// exit_msg & down_msg have the same fields
template<typename T>
typename std::enable_if<
       std::is_same<T, down_msg>::value
    || std::is_same<T, exit_msg>::value
    || std::is_same<T, sync_exited_msg>::value
>::type
serialize_impl(const T& dm, serializer* sink) {
    serialize_impl(dm.source, sink);
    sink->write_value(dm.reason);
}

// exit_msg & down_msg have the same fields
template<typename T>
typename std::enable_if<
       std::is_same<T, down_msg>::value
    || std::is_same<T, exit_msg>::value
    || std::is_same<T, sync_exited_msg>::value
>::type
deserialize_impl(T& dm, deserializer* source) {
    deserialize_impl(dm.source, source);
    dm.reason = source->read<uint32_t>();
}

inline void serialize_impl(const group_down_msg& dm, serializer* sink) {
    serialize_impl(dm.source, sink);
}

inline void deserialize_impl(group_down_msg& dm, deserializer* source) {
    deserialize_impl(dm.source, source);
}

inline void serialize_impl(const message_id& dm, serializer* sink) {
    sink->write_value(dm.integer_value());
}

inline void deserialize_impl(message_id& dm, deserializer* source) {
    dm = message_id::from_integer_value(source->read<uint64_t>());
}

inline void serialize_impl(const timeout_msg& tm, serializer* sink) {
    sink->write_value(tm.timeout_id);
}

inline void deserialize_impl(timeout_msg& tm, deserializer* source) {
    tm.timeout_id = source->read<uint32_t>();
}

inline void serialize_impl(const sync_timeout_msg&, serializer*) { }

inline void deserialize_impl(const sync_timeout_msg&, deserializer*) { }

template<typename Subtype>
inline void serialize_impl(const io_handle<Subtype>& hdl, serializer* sink) {
    sink->write_value(hdl.id());
}

template<typename Subtype>
inline void deserialize_impl(io_handle<Subtype>& hdl, deserializer* source) {
    hdl.set_id(source->read<int64_t>());
}

inline void serialize_impl(const new_connection_msg& msg, serializer* sink) {
    serialize_impl(msg.source, sink);
    serialize_impl(msg.handle, sink);
}

inline void deserialize_impl(new_connection_msg& msg, deserializer* source) {
    deserialize_impl(msg.source, source);
    deserialize_impl(msg.handle, source);
}

inline void serialize_impl(const new_data_msg& msg, serializer* sink) {
    serialize_impl(msg.handle, sink);
    auto buf_size = static_cast<uint32_t>(msg.buf.size());
    if (buf_size != msg.buf.size()) { // narrowing error
        std::ostringstream oss;
        oss << "attempted to send more than "
            << std::numeric_limits<uint32_t>::max()
             << " bytes";
        auto errstr = oss.str();
        BOOST_ACTOR_LOGF_ERROR(errstr);
        throw std::ios_base::failure(std::move(errstr));
    }
    sink->write_value(buf_size);
    sink->write_raw(msg.buf.size(), msg.buf.data());
}

inline void deserialize_impl(new_data_msg& msg, deserializer* source) {
    deserialize_impl(msg.handle, source);
    auto buf_size = source->read<uint32_t>();
    msg.buf.resize(buf_size);
    source->read_raw(msg.buf.size(), msg.buf.data());
}

// exit_msg & down_msg have the same fields
template<typename T>
typename std::enable_if<
       std::is_same<T, connection_closed_msg>::value
    || std::is_same<T, acceptor_closed_msg>::value
>::type
serialize_impl(const T& dm, serializer* sink) {
    serialize_impl(dm.handle, sink);
}

// exit_msg & down_msg have the same fields
template<typename T>
typename std::enable_if<
std::is_same<T, connection_closed_msg>::value
|| std::is_same<T, acceptor_closed_msg>::value
>::type
deserialize_impl(T& dm, deserializer* source) {
    deserialize_impl(dm.handle, source);
}

bool types_equal(const std::type_info* lhs, const std::type_info* rhs) {
    // in some cases (when dealing with dynamic libraries),
    // address can be different although types are equal
    return lhs == rhs || *lhs == *rhs;
}

template<typename T>
class uti_base : public uniform_type_info {

 protected:

    uti_base() : m_native(&typeid(T)) { }

    bool equal_to(const std::type_info& ti) const override {
        return types_equal(m_native, &ti);
    }

    bool equals(const void* lhs, const void* rhs) const override {
        //return detail::safe_equal(deref(lhs), deref(rhs));
        //return deref(lhs) == deref(rhs);
        return eq(deref(lhs), deref(rhs));
    }

    uniform_value create(const uniform_value& other) const override {
        return create_impl<T>(other);
    }

    message as_message(void* instance) const override {
        return make_message(deref(instance));
    }

    static inline const T& deref(const void* ptr) {
        return *reinterpret_cast<const T*>(ptr);
    }

    static inline T& deref(void* ptr) {
        return *reinterpret_cast<T*>(ptr);
    }

    const std::type_info* m_native;

 private:

    template<typename U>
    typename std::enable_if<std::is_floating_point<U>::value, bool>::type
    eq(const U& lhs, const U& rhs) const {
        return detail::safe_equal(lhs, rhs);
    }

    template<typename U>
    typename std::enable_if<!std::is_floating_point<U>::value, bool>::type
    eq(const U& lhs, const U& rhs) const {
        return lhs ==  rhs;
    }

};

template<typename T>
class uti_impl : public uti_base<T> {

    typedef uti_base<T> super;

 public:

    const char* name() const {
        return mapped_name<T>();
    }

 protected:

    void serialize(const void* instance, serializer* sink) const {
        serialize_impl(super::deref(instance), sink);
    }

    void deserialize(void* instance, deserializer* source) const {
        deserialize_impl(super::deref(instance), source);
    }

};

class abstract_int_tinfo : public uniform_type_info {

 public:

    void add_native_type(const std::type_info& ti) {
        // only push back if not already set
        auto predicate = [&](const std::type_info* ptr) { return ptr == &ti; };
        if (std::none_of(m_natives.begin(), m_natives.end(), predicate))
            m_natives.push_back(&ti);
    }

 protected:

    std::vector<const std::type_info*> m_natives;

};

// unfortunately, one integer type can be mapped to multiple types
template<typename T>
class int_tinfo : public abstract_int_tinfo {

 public:

    void serialize(const void* instance, serializer* sink) const override {
        sink->write_value(deref(instance));
    }

    void deserialize(void* instance, deserializer* source) const override {
        deref(instance) = source->read<T>();
    }

    const char* name() const override {
        return static_name();
    }

    message as_message(void* instance) const override {
        return make_message(deref(instance));
    }

 protected:

    bool equal_to(const std::type_info& ti) const override {
        auto tptr = &ti;
        return std::any_of(m_natives.begin(), m_natives.end(),
                           [tptr](const std::type_info* ptr) {
            return types_equal(ptr, tptr);
        });
    }

    bool equals(const void* lhs, const void* rhs) const override {
        return deref(lhs) == deref(rhs);
    }

    uniform_value create(const uniform_value& other) const override {
        return create_impl<T>(other);
    }

 private:

    inline static const T& deref(const void* ptr) {
        return *reinterpret_cast<const T*>(ptr);
    }

    inline static T& deref(void* ptr) {
        return *reinterpret_cast<T*>(ptr);
    }

    static inline const char* static_name() {
        return mapped_int_name<T>();
    }

};

class default_meta_message : public uniform_type_info {

 public:

    default_meta_message(const std::string& name) {
        m_name = name;
        std::vector<std::string> elements;
        split(elements, name, is_any_of("+"));
        auto uti_map = detail::singletons::get_uniform_type_info_map();
        BOOST_ACTOR_REQUIRE(elements.size() > 0 && elements.front() == "@<>");
        // ignore first element, because it's always "@<>"
        for (size_t i = 1; i != elements.size(); ++i) {
            try { m_elements.push_back(uti_map->by_uniform_name(elements[i])); }
            catch (std::exception&) {
                BOOST_ACTOR_LOG_ERROR("type name " << elements[i] << " not found");
            }
        }
    }

    uniform_value create(const uniform_value& other) const override {
        auto res = create_impl<message>(other);
        if (!other) {
            // res is not a copy => fill with values
            message_builder mb;
            for (auto& e : m_elements) mb.append(e->create());
            *cast(res->val) = mb.to_message();
        }
        return res;
    }

    message as_message(void* ptr) const override {
        return *cast(ptr);
    }

    const char* name() const override {
        return m_name.c_str();
    }

    void serialize(const void* ptr, serializer* sink) const override {
        auto& msg = *cast(ptr);
        BOOST_ACTOR_REQUIRE(msg.size() == m_elements.size());
        for (size_t i = 0; i < m_elements.size(); ++i) {
            m_elements[i]->serialize(msg.at(i), sink);
        }
    }

    void deserialize(void* ptr, deserializer* source) const override {
        message_builder mb;
        for (size_t i = 0; i < m_elements.size(); ++i) {
            mb.append(m_elements[i]->deserialize(source));
        }
        *cast(ptr) = mb.to_message();
    }

    bool equal_to(const std::type_info&) const override {
        return false;
    }

    bool equals(const void* instance1, const void* instance2) const override {
        auto& lhs = *cast(instance1);
        auto& rhs = *cast(instance2);
        full_eq_type cmp;
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), cmp);
    }

 private:

    std::string m_name;
    std::vector<const uniform_type_info*> m_elements;

    inline message* cast(void* ptr) const {
        return reinterpret_cast<message*>(ptr);
    }

    inline const message* cast(const void* ptr) const {
        return reinterpret_cast<const message*>(ptr);
    }

};

template<typename T>
void push_native_type(abstract_int_tinfo* m [][2]) {
    m[sizeof(T)][std::is_signed<T>::value ? 1 : 0]->add_native_type(typeid(T));
}

template<typename T0, typename T1, typename... Ts>
void push_native_type(abstract_int_tinfo* m [][2]) {
    push_native_type<T0>(m);
    push_native_type<T1, Ts...>(m);
}

class utim_impl : public uniform_type_info_map {

 public:

    void initialize() {
        // maps sizeof(integer_type) to {signed_type, unsigned_type}
        abstract_int_tinfo* mapping[][2] = {
            {nullptr, nullptr},         // no integer type for sizeof(T) == 0
            {&m_type_u8,  &m_type_i8},
            {&m_type_u16, &m_type_i16},
            {nullptr, nullptr},         // no integer type for sizeof(T) == 3
            {&m_type_u32, &m_type_i32},
            {nullptr, nullptr},         // no integer type for sizeof(T) == 5
            {nullptr, nullptr},         // no integer type for sizeof(T) == 6
            {nullptr, nullptr},         // no integer type for sizeof(T) == 7
            {&m_type_u64, &m_type_i64}
        };
        push_native_type<char,                  signed char,
                         unsigned char,         short,
                         signed short,          unsigned short,
                         short int,             signed short int,
                         unsigned short int,    int,
                         signed int,            unsigned int,
                         long int,              signed long int,
                         unsigned long int,     long,
                         signed long,           unsigned long,
                         long long,             signed long long,
                         unsigned long long,    wchar_t,
                         int8_t,           uint8_t,
                         int16_t,          uint16_t,
                         int32_t,          uint32_t,
                         int64_t,          uint64_t,
                         char16_t,              char32_t,
                         size_t,                ptrdiff_t,
                         intptr_t                                >(mapping);
        // fill builtin types *in sorted order* (by uniform name)
        auto i = m_builtin_types.begin();
        *i++ = &m_type_unit;                // @0
        *i++ = &m_type_accept;              // @accept
        *i++ = &m_type_acceptor_closed;     // @acceptor_closed
        *i++ = &m_type_actor;               // @actor
        *i++ = &m_type_actor_addr;          // @addr
        *i++ = &m_type_atom;                // @atom
        *i++ = &m_type_channel;             // @channel
        *i++ = &m_type_charbuf;             // @charbuf
        *i++ = &m_type_connection;          // @connection
        *i++ = &m_type_connection_closed;   // @connection_closed
        *i++ = &m_type_down_msg;            // @down
        *i++ = &m_type_duration;            // @duration
        *i++ = &m_type_exit_msg;            // @exit
        *i++ = &m_type_group;               // @group
        *i++ = &m_type_group_down;          // @group_down
        *i++ = &m_type_i16;                 // @i16
        *i++ = &m_type_i32;                 // @i32
        *i++ = &m_type_i64;                 // @i64
        *i++ = &m_type_i8;                  // @i8
        *i++ = &m_type_long_double;         // @ldouble
        *i++ = &m_type_message;             // @message
        *i++ = &m_type_message_id;          // @message_id
        *i++ = &m_type_new_connection;      // @new_connection
        *i++ = &m_type_new_data;            // @new_data
        *i++ = &m_type_proc;                // @proc
        *i++ = &m_type_str;                 // @str
        *i++ = &m_type_strmap;              // @strmap
        *i++ = &m_type_sync_exited;         // @sync_exited
        *i++ = &m_type_sync_timeout;        // @sync_timeout
        *i++ = &m_type_timeout;             // @timeout
        *i++ = &m_type_u16;                 // @u16
        *i++ = &m_type_u16str;              // @u16str
        *i++ = &m_type_u32;                 // @u32
        *i++ = &m_type_u32str;              // @u32str
        *i++ = &m_type_u64;                 // @u64
        *i++ = &m_type_u8;                  // @u8
        *i++ = &m_type_bool;                // bool
        *i++ = &m_type_double;              // double
        *i++ = &m_type_float;               // float
        BOOST_ACTOR_REQUIRE(i == m_builtin_types.end());
#       ifdef BOOST_ACTOR_DEBUG_MODE
        using namespace std;
        auto cmp = [](pointer lhs, pointer rhs) {
            return strcmp(lhs->name(), rhs->name()) < 0;
        };
        auto& arr = m_builtin_types;
        if (!is_sorted(arr.begin(), arr.end(), cmp)) {
            cerr << "FATAL: uniform type map not sorted" << endl
                      << "order is:" << endl;
            for (auto ptr : arr) cerr << ptr->name() << endl;
            sort(arr.begin(), arr.end(), cmp);
            cerr << endl << "order should be:" << endl;
            for (auto ptr : arr) cerr << ptr->name() << endl;
            abort();
        }
        auto cmp2 = [](const char** lhs, const char** rhs) {
            return strcmp(lhs[0], rhs[0]) < 0;
        };
        if (!is_sorted(begin(mapped_type_names),
                            end(mapped_type_names), cmp2)) {
            vector<string> mtps;
            for (auto ptr : mapped_type_names) mtps.emplace_back(ptr[0]);
            cerr << "FATAL: mapped_type_names not sorted" << endl
                      << "order is:" << endl;
            for (auto mtp : mtps) cerr << mtp << endl;
            sort(mtps.begin(), mtps.end());
            cerr << endl << "order should be:" << endl;
            for (auto mtp : mtps) cerr << mtp << endl;
            abort();
        }
#       endif
    }

    pointer by_rtti(const std::type_info& ti) const {
        shared_lock<detail::shared_spinlock> guard(m_lock);
        auto res = find_rtti(m_builtin_types, ti);
        return (res) ? res : find_rtti(m_user_types, ti);
    }

    pointer by_uniform_name(const std::string& name) {
        pointer result = nullptr;
        /* lifetime scope of guard */ {
            shared_lock<detail::shared_spinlock> guard(m_lock);
            result = find_name(m_builtin_types, name);
            result = (result) ? result : find_name(m_user_types, name);
        }
        if (!result && name.compare(0, 3, "@<>") == 0) {
            // create tuple UTI on-the-fly
            result = insert(uniform_type_info_ptr{new default_meta_message(name)});
        }
        return result;
    }

    std::vector<pointer> get_all() const {
        shared_lock<detail::shared_spinlock> guard(m_lock);
        std::vector<pointer> res;
        res.reserve(m_builtin_types.size() + m_user_types.size());
        res.insert(res.end(), m_builtin_types.begin(), m_builtin_types.end());
        res.insert(res.end(), m_user_types.begin(), m_user_types.end());
        return res;
    }

    pointer insert(uniform_type_info_ptr uti) {
        unique_lock<detail::shared_spinlock> guard(m_lock);
        auto e = m_user_types.end();
        auto i = std::lower_bound(m_user_types.begin(), e, uti.get(),
                                  [](uniform_type_info* lhs, pointer rhs) {
            return strcmp(lhs->name(), rhs->name()) < 0;
        });
        if (i == e) {
            m_user_types.push_back(uti.release());
            return m_user_types.back();
        }
        else {
            if (strcmp(uti->name(), (*i)->name()) == 0) {
                // type already known
                return *i;
            }
            // insert after lower bound (vector is always sorted)
            auto new_pos = std::distance(m_user_types.begin(), i);
            m_user_types.insert(i, uti.release());
            return m_user_types[static_cast<size_t>(new_pos)];
        }
    }

    ~utim_impl() {
        for (auto ptr : m_user_types) delete ptr;
        m_user_types.clear();
    }

 private:

    typedef std::map<std::string, std::string> strmap;
    typedef std::vector<char> charbuf;

    // 0-9
    uti_impl<node_id_ptr>              m_type_proc;
    uti_impl<channel>                  m_type_channel;
    uti_impl<down_msg>                 m_type_down_msg;
    uti_impl<exit_msg>                 m_type_exit_msg;
    uti_impl<actor>                    m_type_actor;
    uti_impl<actor_addr>               m_type_actor_addr;
    uti_impl<group>                    m_type_group;
    uti_impl<group_down_msg>           m_type_group_down;
    uti_impl<message>                  m_type_message;
    uti_impl<message_id>               m_type_message_id;

    // 10-19
    uti_impl<duration>                 m_type_duration;
    uti_impl<sync_exited_msg>          m_type_sync_exited;
    uti_impl<sync_timeout_msg>         m_type_sync_timeout;
    uti_impl<timeout_msg>              m_type_timeout;
    uti_impl<unit_t>                   m_type_unit;
    uti_impl<atom_value>               m_type_atom;
    uti_impl<std::string>              m_type_str;
    uti_impl<std::u16string>           m_type_u16str;
    uti_impl<std::u32string>           m_type_u32str;
    default_uniform_type_info<strmap>  m_type_strmap;

    // 20-29
    uti_impl<bool>                     m_type_bool;
    uti_impl<float>                    m_type_float;
    uti_impl<double>                   m_type_double;
    uti_impl<long double>              m_type_long_double;
    int_tinfo<int8_t>                  m_type_i8;
    int_tinfo<uint8_t>                 m_type_u8;
    int_tinfo<int16_t>                 m_type_i16;
    int_tinfo<uint16_t>                m_type_u16;
    int_tinfo<int32_t>                 m_type_i32;
    int_tinfo<uint32_t>                m_type_u32;

    // 30-38
    int_tinfo<int64_t>                 m_type_i64;
    int_tinfo<uint64_t>                m_type_u64;
    default_uniform_type_info<charbuf> m_type_charbuf;
    uti_impl<accept_handle>            m_type_accept;
    uti_impl<connection_handle>        m_type_connection;
    uti_impl<acceptor_closed_msg>      m_type_acceptor_closed;
    uti_impl<connection_closed_msg>    m_type_connection_closed;
    uti_impl<new_connection_msg>       m_type_new_connection;
    uti_impl<new_data_msg>             m_type_new_data;

    // both containers are sorted by uniform name
    std::array<pointer, 39> m_builtin_types;
    std::vector<uniform_type_info*> m_user_types;
    mutable detail::shared_spinlock m_lock;

    template<typename Container>
    pointer find_rtti(const Container& c, const std::type_info& ti) const {
        auto e = c.end();
        auto i = std::find_if(c.begin(), e, [&](pointer p) {
            return p->equal_to(ti);
        });
        return (i == e) ? nullptr : *i;
    }

    template<typename Container>
    pointer find_name(const Container& c, const std::string& name) const {
        auto e = c.end();
        // both containers are sorted
        auto i = std::lower_bound(c.begin(), e, name,
                                  [](pointer p, const std::string& n) {
            return p->name() < n;
        });
        return (i != e && (*i)->name() == name) ? *i : nullptr;
    }

};

} // namespace <anonymous>

uniform_type_info_map* uniform_type_info_map::create_singleton() {
    return new utim_impl;
}

uniform_type_info_map::~uniform_type_info_map() { }

} // namespace detail
} // namespace actor
} // namespace boost
