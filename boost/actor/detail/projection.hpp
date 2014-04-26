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


#ifndef BOOST_ACTOR_PROJECTION_HPP
#define BOOST_ACTOR_PROJECTION_HPP

#include "boost/none.hpp"
#include "boost/optional.hpp"

#include "boost/actor/guard_expr.hpp"

#include "boost/actor/detail/call.hpp"
#include "boost/actor/detail/int_list.hpp"
#include "boost/actor/detail/type_list.hpp"
#include "boost/actor/detail/type_traits.hpp"
#include "boost/actor/detail/left_or_right.hpp"

namespace boost {
namespace actor {
namespace detail {

template<typename Fun, typename Tuple, long... Is>
inline bool is_defined_at(Fun& f, Tuple& tup, detail::int_list<Is...>) {
    return f.defined_at(get_cv_aware<Is>(tup)...);
}

template<typename ProjectionFuns, typename... Ts>
struct collected_args_tuple {
    typedef typename detail::tl_apply<
            typename detail::tl_zip<
                typename detail::tl_map<
                    ProjectionFuns,
                    detail::map_to_result_type,
                    detail::rm_optional
                >::type,
                typename detail::tl_map<
                    detail::type_list<Ts...>,
                    mutable_gref_wrapped
                >::type,
                detail::left_or_right
            >::type,
            std::tuple
        >::type
        type;
};

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

/**
 * @brief Projection implemented by a set of functors.
 */
template<class ProjectionFuns, typename... Ts>
class projection {

 public:

    typedef typename detail::tl_apply<ProjectionFuns, std::tuple>::type fun_container;

    typedef detail::type_list<typename detail::rm_const_and_ref<Ts>::type...> arg_types;

    projection() = default;

    projection(fun_container&& args) : m_funs(std::move(args)) { }

    projection(const fun_container& args) : m_funs(args) { }

    projection(const projection&) = default;

    /**
     * @brief Invokes @p fun with a projection of <tt>args...</tt>.
     */
    template<class PartFun>
    optional<typename PartFun::result_type> operator()(PartFun& fun, Ts... args) const {
        typename collected_args_tuple<ProjectionFuns, Ts...>::type pargs;
        auto indices = detail::get_indices(pargs);
        auto args_tup = std::forward_as_tuple(args...);
        collect_visitor cv;
        if (tuple_zip(cv, indices, pargs, m_funs, args_tup)) {
            if (is_defined_at(fun, pargs, indices)) {
                return detail::apply_args(fun, pargs, indices);
            }
        }
        return none;
    }

 private:

    fun_container m_funs;

};

template<>
class projection<detail::empty_type_list> {

 public:

    projection() = default;

    // allow construction from just about everything
    template<typename T>
    explicit projection(const T&) { }

    template<class PartFun>
    optional<typename PartFun::result_type> operator()(PartFun& fun) const {
        if (fun.defined_at()) {
            return fun();
        }
        return none;
    }

};

template<class ProjectionFuns, class List>
struct projection_from_type_list;

template<class ProjectionFuns, typename... Ts>
struct projection_from_type_list<ProjectionFuns, detail::type_list<Ts...> > {
    typedef projection<ProjectionFuns, Ts...> type;
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_PROJECTION_HPP
