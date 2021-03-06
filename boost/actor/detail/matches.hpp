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
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * Distributed under the Boost Software License, Version 1.0. See             *
 * accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt  *
\******************************************************************************/


#ifndef BOOST_ACTOR_DETAIL_MATCHES_HPP
#define BOOST_ACTOR_DETAIL_MATCHES_HPP

#include <numeric>

#include "boost/actor/message.hpp"
#include "boost/actor/wildcard_position.hpp"

#include "boost/actor/detail/type_list.hpp"

namespace boost {
namespace actor {
namespace detail {

template<wildcard_position, class Tuple, typename...>
struct matcher;

template<class Tuple, typename... T>
struct matcher<wildcard_position::nil, Tuple, T...> {
    bool operator()(const Tuple& tup) const {
        if (not tup.dynamically_typed()) {
            // statically typed tuples return &typeid(type_list<T...>)
            // as type token
            return typeid(detail::type_list<T...>) == *(tup.type_token());
        }
        // always use a full dynamic match for dynamic typed tuples
        else if (tup.size() == sizeof...(T)) {
            auto& tarr = static_types_array<T...>::arr;
            return std::equal(tup.begin(), tup.end(), tarr.begin(),
                              types_only_eq);
        }
        return false;
    }
    bool operator()(const Tuple& tup, std::vector<size_t>& mv) const {
        if ((*this)(tup)) {
            mv.resize(sizeof...(T));
            std::iota(mv.begin(), mv.end(), 0);
            return true;
        }
        return false;
    }
};

template<class Tuple, typename... T>
struct matcher<wildcard_position::trailing, Tuple, T...> {
    static constexpr size_t size = sizeof...(T) - 1;
    bool operator()(const Tuple& tup) const {
        if (tup.size() >= size) {
            auto& tarr = static_types_array<T...>::arr;
            auto begin = tup.begin();
            return std::equal(begin, begin + size, tarr.begin(),
                              types_only_eq);
        }
        return false;
    }
    bool operator()(const Tuple& tup, std::vector<size_t>& mv) const {
        if ((*this)(tup)) {
            mv.resize(size);
            std::iota(mv.begin(), mv.end(), 0);
            return true;
        }
        return false;
    }
};

template<class Tuple>
struct matcher<wildcard_position::leading, Tuple, anything> {
    bool operator()(const Tuple&) const {
        return true;
    }
    bool operator()(const Tuple&, std::vector<size_t>&) const {
        return true;
    }
};

template<class Tuple, typename... T>
struct matcher<wildcard_position::leading, Tuple, T...> {
    static constexpr size_t size = sizeof...(T) - 1;
    bool operator()(const Tuple& tup) const {
        auto tup_size = tup.size();
        if (tup_size >= size) {
            auto& tarr = static_types_array<T...>::arr;
            auto begin = tup.begin();
            begin += (tup_size - size);
            return std::equal(begin, tup.end(), (tarr.begin() + 1), // skip 'anything'
                              types_only_eq);
        }
        return false;
    }
    bool operator()(const Tuple& tup, std::vector<size_t>& mv) const {
        if ((*this)(tup)) {
            mv.resize(size);
            std::iota(mv.begin(), mv.end(), tup.size() - size);
            return true;
        }
        return false;
    }
};

template<class Tuple, typename... T>
struct matcher<wildcard_position::in_between, Tuple, T...> {
    static constexpr int signed_wc_pos =
            detail::tl_find<detail::type_list<T...>, anything>::value;
    static constexpr size_t size = sizeof...(T);
    static constexpr size_t wc_pos = static_cast<size_t>(signed_wc_pos);

    static_assert(   signed_wc_pos > 0
                  && wc_pos < (sizeof...(T) - 1),
                  "illegal wildcard position");

    bool operator()(const Tuple& tup) const {
        auto tup_size = tup.size();
        if (tup_size >= (size - 1)) {
            auto& tarr = static_types_array<T...>::arr;
            // first range [0, X1)
            auto begin = tup.begin();
            auto end = begin + wc_pos;
            if (std::equal(begin, end, tarr.begin(), types_only_eq)) {
                // second range [X2, N)
                begin = end = tup.end();
                begin -= (size - (wc_pos + 1));
                auto arr_begin = tarr.begin() + (wc_pos + 1);
                return std::equal(begin, end, arr_begin, types_only_eq);
            }
        }
        return false;
    }

    bool operator()(const Tuple& tup, std::vector<size_t>& mv) const {
        if ((*this)(tup)) {
            // first range
            mv.resize(size - 1);
            auto begin = mv.begin();
            std::iota(begin, begin + wc_pos, 0);
            // second range
            begin = mv.begin() + wc_pos;
            std::iota(begin, mv.end(), tup.size() - (size - (wc_pos + 1)));
            return true;
        }
        return false;
    }
};

template<class Tuple, typename... T>
struct matcher<wildcard_position::multiple, Tuple, T...> {
    static constexpr size_t wc_count =
            detail::tl_count<detail::type_list<T...>, is_anything>::value;

    static_assert(sizeof...(T) > wc_count, "only wildcards given");

    template<class TupleIter, class PatternIter,
             class Push, class Commit, class Rollback>
    bool operator()(TupleIter tbegin, TupleIter tend,
                    PatternIter pbegin, PatternIter pend,
                    Push& push, Commit& commit, Rollback& rollback) const {
        while (!(pbegin == pend && tbegin == tend)) {
            if (pbegin == pend) {
                // reached end of pattern while some values remain unmatched
                return false;
            }
            else if (*pbegin == nullptr) { // nullptr == wildcard (anything)
                // perform submatching
                ++pbegin;
                // always true at the end of the pattern
                if (pbegin == pend) return true;
                // safe current mapping as fallback
                commit();
                // iterate over tuple values until we found a match
                for (; tbegin != tend; ++tbegin) {
                    if (match(tbegin, tend, pbegin, pend,
                              push, commit, rollback)) {
                        return true;
                    }
                    // restore mapping to fallback (delete invalid mappings)
                    rollback();
                }
                return false; // no submatch found
            }
            // compare types
            else if (tbegin.type() == *pbegin) push(tbegin);
            // no match
            else return false;
            // next iteration
            ++tbegin;
            ++pbegin;
        }
        return true; // pbegin == pend && tbegin == tend
    }

    bool operator()(const Tuple& tup) const {
        auto& tarr = static_types_array<T...>::arr;
        if (tup.size() >= (sizeof...(T) - wc_count)) {
            auto fpush = [](const typename Tuple::const_iterator&) { };
            auto fcommit = [] { };
            auto frollback = [] { };
            return match(tup.begin(), tup.end(), tarr.begin(), tarr.end(),
                         fpush, fcommit, frollback);
        }
        return false;
    }

    template<class MappingVector>
    bool operator()(const Tuple& tup, MappingVector& mv) const {
        auto& tarr = static_types_array<T...>::arr;
        if (tup.size() >= (sizeof...(T) - wc_count)) {
            size_t commited_size = 0;
            auto fpush = [&](const typename Tuple::const_iterator& iter) {
                mv.push_back(iter.position());
            };
            auto fcommit = [&] {
                commited_size = mv.size();
            };
            auto frollback = [&] {
                mv.resize(commited_size);
            };
            return match(tup.begin(), tup.end(),
                         tarr.begin(), tarr.end(),
                         fpush, fcommit, frollback);
        }
        return false;
    }
};

template<class Tuple, class List>
struct select_matcher;

template<class Tuple, typename... Ts>
struct select_matcher<Tuple, detail::type_list<Ts...> > {
    typedef matcher<get_wildcard_position<detail::type_list<Ts...>>(),
                    Tuple,
                    Ts...>
            type;
};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_DETAIL_MATCHES_HPP
