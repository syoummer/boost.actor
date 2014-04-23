#ifndef TEST_HPP
#define TEST_HPP

#include <vector>
#include <string>
#include <cstring>
#include <cstddef>
#include <sstream>
#include <iostream>
#include <type_traits>

#include "boost/actor/cppa.hpp"
#include "boost/actor/actor.hpp"
#include "boost/actor/config.hpp"
#include "boost/actor/logging.hpp"
#include "boost/actor/to_string.hpp"
#include "boost/actor/util/scope_guard.hpp"

#ifndef BOOST_ACTOR_WINDOWS
constexpr char to_dev_null[] = " &>/dev/null";
#else
constexpr char to_dev_null[] = "";
#endif // BOOST_ACTOR_WINDOWS

void set_default_test_settings();

size_t cppa_error_count();
void cppa_inc_error_count();
std::string cppa_fill4(size_t value);
const char* cppa_strip_path(const char* file);
void cppa_unexpected_message(const char* file, size_t line, boost::actor::any_tuple t);
void cppa_unexpected_timeout(const char* file, size_t line);

#define BOOST_ACTOR_STREAMIFY(fname, line, message)                                   \
    cppa_strip_path(fname) << ":" << cppa_fill4(line) << " " << message

#define BOOST_ACTOR_PRINTC(filename, linenum, message)                                \
    BOOST_ACTOR_LOGF_INFO(BOOST_ACTOR_STREAMIFY(filename, linenum, message));                \
    std::cout << BOOST_ACTOR_STREAMIFY(filename, linenum, message) << std::endl

#define BOOST_ACTOR_PRINT(message) BOOST_ACTOR_PRINTC(__FILE__, __LINE__, message)

#define BOOST_ACTOR_PRINTERRC(fname, linenum, msg)                                    \
    BOOST_ACTOR_LOGF(BOOST_ACTOR_ERROR, BOOST_ACTOR_STREAMIFY(fname, linenum, msg));                \
    std::cerr << "ERROR: " << BOOST_ACTOR_STREAMIFY(fname, linenum, msg)              \
              << std::endl

#define BOOST_ACTOR_PRINTERR(message) BOOST_ACTOR_PRINTERRC(__FILE__, __LINE__, message)

template<typename T1, typename T2>
struct both_integral {
    static constexpr bool value =    std::is_integral<T1>::value
                                  && std::is_integral<T2>::value;
};

template<bool V, typename T1, typename T2>
struct enable_integral : std::enable_if<   both_integral<T1, T2>::value == V
                                        && not std::is_pointer<T1>::value
                                        && not std::is_pointer<T2>::value> { };

template<typename T>
const T& cppa_stream_arg(const T& value) {
    return value;
}

inline std::string cppa_stream_arg(const boost::actor::actor& ptr) {
    return boost::actor::to_string(ptr);
}

inline std::string cppa_stream_arg(const boost::actor::actor_addr& ptr) {
    return boost::actor::to_string(ptr);
}

inline std::string cppa_stream_arg(const bool& value) {
    return value ? "true" : "false";
}

inline void cppa_passed(const char* fname, size_t line_number) {
    BOOST_ACTOR_PRINTC(fname, line_number, "passed");
}

template<typename V1, typename V2>
inline void cppa_failed(const V1& v1,
                        const V2& v2,
                        const char* fname,
                        size_t line_number) {
    BOOST_ACTOR_PRINTERRC(fname, line_number,
                   "expected value: " << cppa_stream_arg(v2)
                   << ", found: " << cppa_stream_arg(v1));
    cppa_inc_error_count();
}

inline void cppa_check_value(const std::string& v1,
                             const std::string& v2,
                             const char* fname,
                             size_t line,
                             bool expected = true) {
    if ((v1 == v2) == expected) cppa_passed(fname, line);
    else cppa_failed(v1, v2, fname, line);
}

template<typename V1, typename V2>
inline void cppa_check_value(const V1& v1,
                             const V2& v2,
                             const char* fname,
                             size_t line,
                             bool expected = true,
                             typename enable_integral<false, V1, V2>::type* = 0) {
    if (boost::actor::detail::safe_equal(v1, v2) == expected) cppa_passed(fname, line);
    else cppa_failed(v1, v2, fname, line);
}

template<typename V1, typename V2>
inline void cppa_check_value(V1 v1,
                             V2 v2,
                             const char* fname,
                             size_t line,
                             bool expected = true,
                             typename enable_integral<true, V1, V2>::type* = 0) {
    if ((v1 == static_cast<V1>(v2)) == expected) cppa_passed(fname, line);
    else cppa_failed(v1, v2, fname, line);
}

#define BOOST_ACTOR_VERBOSE_EVAL(LineOfCode)                                          \
    BOOST_ACTOR_PRINT(#LineOfCode << " = " << (LineOfCode));

#define BOOST_ACTOR_TEST(testname)                                             \
    auto cppa_test_scope_guard = ::boost::actor::util::make_scope_guard([] {   \
        std::cout << cppa_error_count() << " error(s) detected" << std::endl;  \
    });                                                                        \
    set_default_test_settings();                                               \
    BOOST_ACTOR_LOGF_INFO("run unit test " << #testname)

#define BOOST_ACTOR_TEST_RESULT() ((cppa_error_count() == 0) ? 0 : -1)

#define BOOST_ACTOR_CHECK_VERBOSE(line_of_code, err_stream)                           \
    if (!(line_of_code)) {                                                     \
        std::cerr << err_stream << std::endl;                                  \
        cppa_inc_error_count();                                                \
    }                                                                          \
    else {                                                                     \
        BOOST_ACTOR_PRINT("passed");                                                  \
    } ((void) 0)

#define BOOST_ACTOR_CHECK(line_of_code)                                               \
    if (!(line_of_code)) {                                                     \
        BOOST_ACTOR_PRINTERR(#line_of_code);                                          \
        cppa_inc_error_count();                                                \
    }                                                                          \
    else { BOOST_ACTOR_PRINT("passed"); } BOOST_ACTOR_VOID_STMT

#define BOOST_ACTOR_CHECK_EQUAL(lhs_loc, rhs_loc)                                     \
    cppa_check_value((lhs_loc), (rhs_loc), __FILE__, __LINE__)

#define BOOST_ACTOR_CHECK_NOT_EQUAL(rhs_loc, lhs_loc)                                 \
    cppa_check_value((lhs_loc), (rhs_loc), __FILE__, __LINE__, false)

#define BOOST_ACTOR_FAILURE(err_msg) {                                                \
        BOOST_ACTOR_PRINTERR("ERROR: " << err_msg);                                   \
        cppa_inc_error_count();                                                \
    } ((void) 0)

#define BOOST_ACTOR_CHECKPOINT()                                                      \
    BOOST_ACTOR_PRINT("passed")

#define BOOST_ACTOR_UNEXPECTED_TOUT()                                                 \
    cppa_unexpected_timeout(__FILE__, __LINE__)

#define BOOST_ACTOR_UNEXPECTED_MSG(selfptr)                                           \
    cppa_unexpected_message(__FILE__, __LINE__, selfptr ->last_dequeued())

// some convenience macros for defining callbacks
#define BOOST_ACTOR_CHECKPOINT_CB() [] { BOOST_ACTOR_CHECKPOINT(); }
#define BOOST_ACTOR_FAILURE_CB(err_msg) [] { BOOST_ACTOR_FAILURE(err_msg); }
#define BOOST_ACTOR_UNEXPECTED_MSG_CB(selfptr) [=] { BOOST_ACTOR_UNEXPECTED_MSG(selfptr); }
#define BOOST_ACTOR_UNEXPECTED_MSG_CB_REF(selfref) [&] { BOOST_ACTOR_UNEXPECTED_MSG(selfref); }
#define BOOST_ACTOR_UNEXPECTED_TOUT_CB() [] { BOOST_ACTOR_UNEXPECTED_TOUT(); }

std::vector<std::string> split(const std::string& str, char delim = ' ', bool keep_empties = true);

std::map<std::string, std::string> get_kv_pairs(int argc, char** argv, int begin = 1);

template<typename F>
void run_client_part(const std::map<std::string, std::string>& args, F fun) {
    BOOST_ACTOR_LOGF_INFO("run in client mode");
    auto i = args.find("port");
    if (i == args.end()) {
        BOOST_ACTOR_LOGF_ERROR("no port specified");
        throw std::logic_error("no port specified");
    }
    auto port = static_cast<std::uint16_t>(stoi(i->second));
    fun(port);
    boost::actor::await_all_actors_done();
    boost::actor::shutdown();
}

#endif // TEST_HPP
