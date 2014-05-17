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


#include "boost/none.hpp"

#include "boost/actor/behavior.hpp"
#include "boost/actor/message_handler.hpp"

namespace boost {
namespace actor {

namespace {
class continuation_decorator : public detail::behavior_impl {

    typedef behavior_impl super;

 public:

    typedef behavior::continuation_fun continuation_fun;

    typedef typename behavior_impl::pointer pointer;

    continuation_decorator(continuation_fun fun, pointer ptr)
    : super(ptr->timeout()), m_fun(fun), m_decorated(std::move(ptr)) {
        BOOST_ACTOR_REQUIRE(m_decorated != nullptr);
    }

    template<typename T>
    inline bhvr_invoke_result invoke_impl(T& tup) {
        auto res = m_decorated->invoke(tup);
        if (res) return m_fun(*res);
        return none;
    }

    bhvr_invoke_result invoke(message& tup) {
        return invoke_impl(tup);
    }

    bhvr_invoke_result invoke(const message& tup) {
        return invoke_impl(tup);
    }

    pointer copy(const generic_timeout_definition& tdef) const {
        return new continuation_decorator(m_fun, m_decorated->copy(tdef));
    }

    void handle_timeout() { m_decorated->handle_timeout(); }

 private:

    continuation_fun m_fun;
    pointer m_decorated;

};
} // namespace <anonymous>

behavior::behavior(const message_handler& fun) : m_impl(fun.m_impl) { }

behavior behavior::add_continuation(continuation_fun fun) {
    return behavior::impl_ptr{new continuation_decorator(std::move(fun),
                                                         m_impl)};
}

} // namespace actor
} // namespace boost
