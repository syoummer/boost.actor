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


#ifndef BOOST_ACTOR_SINGLETON_MIXIN_HPP
#define BOOST_ACTOR_SINGLETON_MIXIN_HPP

#include <utility>

namespace boost {
namespace actor {
namespace detail {

class singleton_manager;

// a mixin for simple singleton classes
template<class Derived, class Base = void>
class singleton_mixin : public Base {

    friend class singleton_manager;

    inline static Derived* create_singleton() { return new Derived; }
    inline void dispose() { delete this; }
    inline void destroy() { delete this; }
    inline void initialize() { }

 protected:

    template<typename... Ts>
    singleton_mixin(Ts&&... args) : Base(std::forward<Ts>(args)...) { }

    virtual ~singleton_mixin() { }

};

template<class Derived>
class singleton_mixin<Derived, void> {

    friend class singleton_manager;

    inline static Derived* create_singleton() { return new Derived; }
    inline void dispose() { delete this; }
    inline void destroy() { delete this; }
    inline void initialize() { }

 protected:

    virtual ~singleton_mixin() { }

};

} // namespace detail
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_SINGLETON_MIXIN_HPP
