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


#ifndef BOOST_ACTOR_TYPED_CONTINUE_HELPER_HPP
#define BOOST_ACTOR_TYPED_CONTINUE_HELPER_HPP

#include "boost/actor/continue_helper.hpp"

#include "boost/actor/detail/type_traits.hpp"

#include "boost/actor/detail/typed_actor_util.hpp"

namespace boost {
namespace actor {

template<typename OutputList>
class typed_continue_helper {

 public:

    typedef int message_id_wrapper_tag;

    typed_continue_helper(message_id mid, local_actor* self) : m_ch(mid, self) { }

    template<typename F>
    typed_continue_helper<typename detail::get_callable_trait<F>::result_type>
    continue_with(F fun) {
        detail::assert_types<OutputList, F>();
        m_ch.continue_with(std::move(fun));
        return {m_ch};
    }

    inline message_id get_message_id() const {
        return m_ch.get_message_id();
    }

    typed_continue_helper(continue_helper ch) : m_ch(std::move(ch)) { }

 private:

    continue_helper m_ch;

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_TYPED_CONTINUE_HELPER_HPP
