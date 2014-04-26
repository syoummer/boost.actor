/******************************************************************************\
 *           ___        __                                                    *
 *          /\_ \    __/\ \                                                   *
 *          \//\ \  /\_\ \ \____    ___   _____   _____      __               *
 *            \ \ \ \/\ \ \ '__`\  /'___\/\ '__`\/\ '__`\  /'__`\             *
 *             \_\ \_\ \ \ \ \L\ \/\ \__/\ \ \L\ \ \ \L\ \/\ \L\.\_           *
 *             /\____\\ \_\ \_,__/\ \____\\ \ ,__/\ \ ,__/\ \__/.\_\          *
 *             \/____/ \/_/\/___/  \/____/ \ \ \/  \ \ \/  \/__/\/_/          *
 *                                          \ \_\   \ \_\                     *
 *                                           \/_/    \/_/                     *
 *                                                                            *
 * Copyright (C) 2011-2013                                                    *
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * This file is part of libcppa.                                              *
 * libcppa is free software: you can redistribute it and/or modify it under   *
 * the terms of the GNU Lesser General Public License as published by the     *
 * Free Software Foundation; either version 2.1 of the License,               *
 * or (at your option) any later version.                                     *
 *                                                                            *
 * libcppa is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with libcppa. If not, see <http://www.gnu.org/licenses/>.            *
\******************************************************************************/


#ifndef BOOST_ACTOR_ON_HPP
#define BOOST_ACTOR_ON_HPP

#include <chrono>
#include <memory>
#include <type_traits>

#include "boost/actor/unit.hpp"
#include "boost/actor/atom.hpp"
#include "boost/actor/anything.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/duration.hpp"
#include "boost/actor/guard_expr.hpp"
#include "boost/actor/match_expr.hpp"
#include "boost/actor/skip_message.hpp"
#include "boost/actor/may_have_timeout.hpp"
#include "boost/actor/timeout_definition.hpp"

#include "boost/actor/detail/type_list.hpp"
#include "boost/actor/detail/arg_match_t.hpp"
#include "boost/actor/detail/type_traits.hpp"

#include "boost/actor/detail/boxed.hpp"
#include "boost/actor/detail/unboxed.hpp"
#include "boost/actor/detail/value_guard.hpp"
#include "boost/actor/detail/implicit_conversions.hpp"

namespace boost {
namespace actor {
namespace detail {

template<bool IsFun, typename T>
struct add_ptr_to_fun_ { typedef T* type; };

template<typename T>
struct add_ptr_to_fun_<false, T> { typedef T type; };

template<typename T>
struct add_ptr_to_fun : add_ptr_to_fun_<std::is_function<T>::value, T> { };

template<bool ToVoid, typename T>
struct to_void_impl { typedef unit_t type; };

template<typename T>
struct to_void_impl<false, T> { typedef typename add_ptr_to_fun<T>::type type; };

template<typename T>
struct boxed_and_not_callable_to_void
: to_void_impl<is_boxed<T>::value || !detail::is_callable<T>::value, T> { };

template<typename T>
struct boxed_and_callable_to_void
: to_void_impl<is_boxed<T>::value || detail::is_callable<T>::value, T> { };

class behavior_rvalue_builder {

 public:

    constexpr behavior_rvalue_builder(const duration& d) : m_tout(d) { }

    template<typename F>
    timeout_definition<F> operator>>(F&& f) const {
        return {m_tout, std::forward<F>(f)};
    }

 private:

    duration m_tout;

};

struct rvalue_builder_args_ctor { };

template<class Left, class Right>
struct disjunct_rvalue_builders {

 public:

    disjunct_rvalue_builders(Left l, Right r)
    : m_left(std::move(l)), m_right(std::move(r)) { }

    template<typename Expr>
    auto operator>>(Expr expr)
         -> decltype((*(static_cast<Left*>(nullptr)) >> expr).or_else(
                      *(static_cast<Right*>(nullptr)) >> expr)) const {
        return (m_left >> expr).or_else(m_right >> expr);
    }

 private:

    Left m_left;
    Right m_right;

};

template<class Guard, class Transformers, class Pattern>
struct rvalue_builder {

    typedef typename detail::tl_back<Pattern>::type back_type;

    static constexpr bool is_complete =
            !std::is_same<detail::arg_match_t, back_type>::value;

    typedef typename detail::tl_apply<Transformers, std::tuple>::type fun_container;

    Guard m_guard;
    fun_container m_funs;

 public:

    rvalue_builder() = default;

    template<typename... Ts>
    rvalue_builder(rvalue_builder_args_ctor, const Ts&... args)
    : m_guard(args...), m_funs(args...) { }

    rvalue_builder(Guard arg0, fun_container arg1)
    : m_guard(std::move(arg0)), m_funs(std::move(arg1)) { }

    template<typename NewGuard>
    rvalue_builder<
        guard_expr<
            logical_and_op,
            guard_expr<exec_xfun_op, Guard, unit_t>,
            NewGuard>,
        Transformers,
        Pattern>
    when(NewGuard ng,
         typename std::enable_if<
               std::is_same<NewGuard, NewGuard>::value
            && !std::is_same<Guard, empty_value_guard>::value
         >::type* = 0                                 ) const {
        return {(ge_sub_function(m_guard) && ng), std::move(m_funs)};
    }

    template<typename NewGuard>
    rvalue_builder<NewGuard, Transformers, Pattern>
    when(NewGuard ng,
         typename std::enable_if<
               std::is_same<NewGuard, NewGuard>::value
            && std::is_same<Guard, empty_value_guard>::value
         >::type* = 0                                       ) const {
        return {std::move(ng), std::move(m_funs)};
    }

    template<typename Expr>
    match_expr<typename get_case<is_complete, Expr, Guard, Transformers, Pattern>::type>
    operator>>(Expr expr) const {
        typedef typename get_case<
                    is_complete,
                    Expr,
                    Guard,
                    Transformers,
                    Pattern
                >::type
                tpair;
        return tpair{typename tpair::first_type{m_funs},
                     typename tpair::second_type{std::move(expr),
                                                 std::move(m_guard)}};
    }

    template<class G, class T, class P>
    disjunct_rvalue_builders<rvalue_builder, rvalue_builder<G, T, P> >
    operator||(rvalue_builder<G, T, P> other) const {
        return {*this, std::move(other)};
    }

};

template<bool IsCallable, typename T>
struct pattern_type_ {
    typedef detail::get_callable_trait<T> ctrait;
    typedef typename ctrait::arg_types args;
    static_assert(detail::tl_size<args>::value == 1, "only unary functions allowed");
    typedef typename detail::rm_const_and_ref<typename detail::tl_head<args>::type>::type type;
};

template<typename T>
struct pattern_type_<false, T> {
    typedef typename implicit_conversions<
                typename detail::rm_const_and_ref<
                    typename detail::unboxed<T>::type
                >::type
            >::type
            type;
};

template<typename T>
struct pattern_type : pattern_type_<detail::is_callable<T>::value && !detail::is_boxed<T>::value, T> {
};

} // namespace detail
} // namespace actor
} // namespace boost

namespace boost {
namespace actor {

/**
 * @brief A wildcard that matches any number of any values.
 */
constexpr anything any_vals = anything{};

#ifdef BOOST_ACTOR_DOCUMENTATION

/**
 * @brief A wildcard that matches the argument types
 *        of a given callback. Must be the last argument to {@link on()}.
 * @see {@link math_actor_example.cpp Math Actor Example}
 */
constexpr __unspecified__ arg_match;

/**
 * @brief Left-hand side of a partial function expression.
 *
 * Equal to <tt>on(arg_match)</tt>.
 */
constexpr __unspecified__ on_arg_match;

/**
 * @brief A wildcard that matches any value of type @p T.
 * @see {@link math_actor_example.cpp Math Actor Example}
 */
template<typename T>
__unspecified__ val();

/**
 * @brief Left-hand side of a partial function expression that matches values.
 *
 * This overload can be used with the wildcards {@link cppa::val val},
 * {@link cppa::any_vals any_vals} and {@link cppa::arg_match arg_match}.
 */
template<typename T, typename... Ts>
__unspecified__ on(const T& arg, const Ts&... args);

/**
 * @brief Left-hand side of a partial function expression that matches types.
 *
 * This overload matches types only. The type {@link cppa::anything anything}
 * can be used as wildcard to match any number of elements of any types.
 */
template<typename... Ts>
__unspecified__ on();

/**
 * @brief Left-hand side of a partial function expression that matches types.
 *
 * This overload matches up to four leading atoms.
 * The type {@link cppa::anything anything}
 * can be used as wildcard to match any number of elements of any types.
 */
template<atom_value... Atoms, typename... Ts>
__unspecified__ on();

/**
 * @brief Converts @p arg to a match expression by returning
 *        <tt>on_arg_match >> arg</tt> if @p arg is a callable type,
 *        otherwise returns @p arg.
 */
template<typename T>
__unspecified__ lift_to_match_expr(T arg);

#else

template<typename T>
constexpr typename detail::boxed<T>::type val() {
    return typename detail::boxed<T>::type();
}

typedef typename detail::boxed<detail::arg_match_t>::type boxed_arg_match_t;

constexpr boxed_arg_match_t arg_match = boxed_arg_match_t();

template<typename T, typename... Ts>
detail::rvalue_builder<
    detail::value_guard<
        typename detail::tl_filter_not<
            typename detail::tl_trim<
                typename detail::tl_map<
                    detail::type_list<T, Ts...>,
                    detail::boxed_and_callable_to_void,
                    detail::implicit_conversions
                >::type
            >::type,
            is_anything
        >::type
    >,
    typename detail::tl_map<
        detail::type_list<T, Ts...>,
        detail::boxed_and_not_callable_to_void
    >::type,
    detail::type_list<typename detail::pattern_type<T>::type,
                    typename detail::pattern_type<Ts>::type...> >
on(const T& arg, const Ts&... args) {
    return {detail::rvalue_builder_args_ctor{}, arg, args...};
}

inline detail::rvalue_builder<detail::empty_value_guard,
                              detail::empty_type_list,
                              detail::empty_type_list     > on() {
    return {};
}

template<typename T0, typename... Ts>
detail::rvalue_builder<detail::empty_value_guard,
                       detail::empty_type_list,
                       detail::type_list<T0, Ts...> >
on() {
    return {};
}

template<atom_value A0, typename... Ts>
decltype(on(A0, val<Ts>()...)) on() {
    return on(A0, val<Ts>()...);
}

template<atom_value A0, atom_value A1, typename... Ts>
decltype(on(A0, A1, val<Ts>()...)) on() {
    return on(A0, A1, val<Ts>()...);
}

template<atom_value A0, atom_value A1, atom_value A2, typename... Ts>
decltype(on(A0, A1, A2, val<Ts>()...)) on() {
    return on(A0, A1, A2, val<Ts>()...);
}

template<atom_value A0, atom_value A1, atom_value A2, atom_value A3,
         typename... Ts>
decltype(on(A0, A1, A2, A3, val<Ts>()...)) on() {
    return on(A0, A1, A2, A3, val<Ts>()...);
}

template<class Rep, class Period>
constexpr detail::behavior_rvalue_builder
after(const std::chrono::duration<Rep, Period>& d) {
    return { duration(d) };
}

inline decltype(on<anything>()) others() {
    return on<anything>();
}

// some more convenience

namespace detail {

class on_the_fly_rvalue_builder {

 public:

    constexpr on_the_fly_rvalue_builder() { }

    template<typename Guard>
    auto when(Guard g) const -> decltype(on(arg_match).when(g)) {
        return on(arg_match).when(g);
    }

    template<typename Expr>
    match_expr<
        typename get_case<
            false,
            Expr,
            empty_value_guard,
            detail::empty_type_list,
            detail::empty_type_list
        >::type>
    operator>>(Expr expr) const {
        typedef typename get_case<
                    false,
                    Expr,
                    empty_value_guard,
                    detail::empty_type_list,
                    detail::empty_type_list
                >::type
                result_type;
        return result_type{typename result_type::first_type{},
                           typename result_type::second_type{
                               std::move(expr),
                               empty_value_guard{}}};
    }

};

} // namespace detail

constexpr detail::on_the_fly_rvalue_builder on_arg_match;

#endif // BOOST_ACTOR_DOCUMENTATION

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_ON_HPP
