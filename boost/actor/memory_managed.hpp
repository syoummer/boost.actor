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


#ifndef BOOST_ACTOR_MEMORY_MANAGED_HPP
#define BOOST_ACTOR_MEMORY_MANAGED_HPP

namespace boost {
namespace actor {

namespace detail { struct disposer; }

/**
 * @brief This base enables derived classes to enforce a different
 *        allocation strategy than new/delete by providing a virtual
 *        protected @p request_deletion() function and non-public destructor.
 */
class memory_managed {

    friend struct detail::disposer;

 protected:

    virtual ~memory_managed();

    /**
     * @brief Default implementations calls <tt>delete this</tt>, but can
     *        be overriden in case deletion depends on some condition or
     *        the class doesn't use default new/delete.
     */
    virtual void request_deletion();

};

} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_MEMORY_MANAGED_HPP
