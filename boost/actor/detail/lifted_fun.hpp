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


#ifndef BOOST_ACTOR_DETAIL_LIFTED_FUN_HPP
#define BOOST_ACTOR_DETAIL_LIFTED_FUN_HPP

#include "boost/none.hpp"
#include "boost/optional.hpp"

#include "boost/actor/skip_message.hpp"

#include "boost/actor/detail/call.hpp"
#include "boost/actor/detail/int_list.hpp"
#include "boost/actor/detail/type_list.hpp"
#include "boost/actor/detail/tuple_zip.hpp"
#include "boost/actor/detail/type_traits.hpp"
#include "boost/actor/detail/left_or_right.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename Fun>
struct is_void_fun {
    static constexpr bool value = std::is_same<typename Fun::result_type, void>::value;
};

struct collect_visitor {
    typedef bool result_type;
    inline bool operator()() const {
        // end of recursion
        return true;
    }
    template<typename Storage, typename T>
    static inline  bool store(Storage& storage, T&& value) {
        storage = std::forward<T>(value);
        return true;
    }

    template<class Storage>
    static inline bool store(Storage& storage, optional<Storage>&& value) {
        if (value) {
            storage = std::move(*value);
            return true;
        }
        return false;
    }

    template<typename T>
    static inline auto fetch(const unit_t&, T&& arg)
    -> decltype(std::forward<T>(arg)) {
        return std::forward<T>(arg);
    }

    template<typename Fun, typename T>
    static inline auto fetch(const Fun& fun, T&& arg)
    -> decltype(fun(std::forward<T>(arg))) {
        return fun(std::forward<T>(arg));
    }
    template<typename T0, typename T1, typename T2, typename... Vs>
    inline bool operator()(std::tuple<T0, T1, T2> fwd, Vs&&... args) const {
        return    store(std::get<0>(fwd), fetch(std::get<1>(fwd), std::get<2>(fwd)))
               && (*this)(args...);
    }
};

class lifted_fun_zipper {
 public:
    template<typename F, typename T>
    auto operator()(const F& fun, T&& arg) -> decltype(fun(std::forward<T>(arg))) const {
        return fun(std::forward<T>(arg));
    }
    template<typename T>
    auto operator()(const unit_t&, T&& arg) -> decltype(std::forward<T>(arg)) const {
        return std::forward<T>(arg);
    }
};

template<typename T>
struct is_optional : std::false_type { };

template<typename T>
struct is_optional<optional<T>> : std::true_type { };

template<typename T>
typename std::enable_if<not is_optional<typename rm_const_and_ref<T>::type>::value, T&&>::type
fwd_and_unopt(typename std::remove_reference<T>::type& v) {
    return static_cast<T&&>(v);
}

template<typename T>
auto fwd_and_unopt(typename std::remove_reference<T>::type& v) ->
typename std::enable_if<is_optional<typename rm_const_and_ref<T>::type>::value, decltype(*v)>::type {
    return *v;
}

inline bool has_none() {
    return false;
}

template<typename T, typename... Ts>
bool has_none(const T&, const Ts&... vs) {
    return has_none(vs...);
}

template<typename T, typename... Ts>
bool has_none(const optional<T>& v, const Ts&... vs) {
    return !v || has_none(vs...);
}

// allows F to have fewer arguments than the lifted_fun calling it
template<typename R, typename F>
class lifted_fun_invoker {

    typedef typename get_callable_trait<F>::arg_types arg_types;

    static constexpr size_t args = tl_size<arg_types>::value;

 public:

    lifted_fun_invoker(F& fun) : f(fun) {  }

    template<typename... Ts>
    typename std::enable_if<sizeof...(Ts) == args, R>::type
    operator()(Ts&&... args) const {
        if (has_none(args...)) return none;
        return f(fwd_and_unopt<Ts>(args)...);
    }

    template<typename T, typename... Ts>
    typename std::enable_if<(sizeof...(Ts) + 1 > args), R>::type
    operator()(T&& arg, Ts&&... args) const {
        if (has_none(arg)) return none;
        return (*this)(fwd_and_unopt<Ts>(args)...);
    }

 private:

    F& f;

};

template<typename F>
class lifted_fun_invoker<bool, F> {

    typedef typename get_callable_trait<F>::arg_types arg_types;

    static constexpr size_t args = tl_size<arg_types>::value;

 public:

    lifted_fun_invoker(F& fun) : f(fun) {  }

    template<typename... Ts>
    typename std::enable_if<sizeof...(Ts) == args, bool>::type
    operator()(Ts&&... args) const {
        if (has_none(args...)) return false;
        f(fwd_and_unopt<Ts>(args)...);
        return true;
    }

    template<typename T, typename... Ts>
    typename std::enable_if<(sizeof...(Ts) + 1 > args), bool>::type
    operator()(T&& arg, Ts&&... args) const {
        if (has_none(arg)) return none;
        return (*this)(fwd_and_unopt<Ts>(args)...);
    }

 private:

    F& f;

};


/**
 * @brief A lifted functor consists of a set of projections, a plain-old
 *        functor and its signature. Note that the signature of the lifted
 *        functor might differ from the underlying functor, because
 *        of the projections.
 */
template<typename F, class ListOfProjections, typename... Args>
class lifted_fun {

 public:

    typedef typename get_callable_trait<F>::result_type result_type;

    // Let F be "R (Ts...)" then lifted_fun<F...> returns optional<R>
    // unless R is void in which case bool is returned
    typedef typename std::conditional<
                std::is_same<result_type, void>::value,
                bool,
                optional<result_type>
            >::type
            optional_result_type;

    typedef ListOfProjections projections_list;

    typedef typename detail::tl_apply<projections_list, std::tuple>::type
            projections;

    typedef detail::type_list<Args...> arg_types;

    lifted_fun() = default;

    lifted_fun(lifted_fun&&) = default;

    lifted_fun(const lifted_fun&) = default;

    lifted_fun& operator=(lifted_fun&&) = default;

    lifted_fun& operator=(const lifted_fun&) = default;

    lifted_fun(F f) : m_fun(std::move(f)) { }

    lifted_fun(F f, projections ps) : m_fun(std::move(f)), m_ps(std::move(ps)) { }

    /**
     * @brief Invokes @p fun with a lifted_fun of <tt>args...</tt>.
     */
    optional_result_type operator()(Args... args) {
        auto indices = detail::get_indices(m_ps);
        lifted_fun_zipper zip;
        lifted_fun_invoker<optional_result_type, F> invoke{m_fun};
        auto fwd_args = std::forward_as_tuple(args...);
        return detail::apply_args(invoke, detail::tuple_zip(zip, indices, m_ps, fwd_args), indices);
    }

 private:

    F m_fun;
    projections m_ps;

};

template<typename F, class ListOfProjections, class List>
struct get_lifted_fun;

template<typename F, class ListOfProjections, typename... Ts>
struct get_lifted_fun<F, ListOfProjections, detail::type_list<Ts...> > {
    typedef lifted_fun<F, ListOfProjections, Ts...> type;
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_LIFTED_FUN_HPP
