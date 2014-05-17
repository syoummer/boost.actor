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


#ifndef CONTINUE_HELPER_HPP
#define CONTINUE_HELPER_HPP

#include <functional>

#include "boost/actor/on.hpp"
#include "boost/actor/behavior.hpp"
#include "boost/actor/message_id.hpp"
#include "boost/actor/message_handler.hpp"

namespace boost {
namespace actor {

class local_actor;

/**
 * @brief Helper class to enable users to add continuations
 *        when dealing with synchronous sends.
 */
class continue_helper {

 public:

    typedef int message_id_wrapper_tag;

    continue_helper(message_id mid, local_actor* self);

    /**
     * @brief Adds a continuation to the synchronous message handler
     *        that is invoked if the response handler successfully returned.
     * @param fun The continuation as functor object.
     */
    template<typename F>
    continue_helper& continue_with(F fun) {
        return continue_with(behavior::continuation_fun{message_handler{
                   on(any_vals, arg_match) >> fun
               }});
    }

    /**
     * @brief Adds a continuation to the synchronous message handler
     *        that is invoked if the response handler successfully returned.
     * @param fun The continuation as functor object.
     */
    continue_helper& continue_with(behavior::continuation_fun fun);

    /**
     * @brief Returns the ID of the expected response message.
     */
    message_id get_message_id() const {
        return m_mid;
    }

 private:

    message_id m_mid;
    local_actor* m_self;

};

} // namespace actor
} // namespace boost

#endif // CONTINUE_HELPER_HPP
