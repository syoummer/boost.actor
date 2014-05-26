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


#ifndef BOOST_ACTOR_PRIORITY_POLICY_HPP
#define BOOST_ACTOR_PRIORITY_POLICY_HPP

namespace boost { namespace actor { class mailbox_element; } }

namespace boost {
namespace actor {
namespace policy {

/**
 * @brief The priority_policy <b>concept</b> class. Please note that this
 *        class is <b>not</b> implemented. It only explains the all
 *        required member function and their behavior for any priority policy.
 */
class priority_policy {

 public:

    /**
     * @brief Returns the next message from the mailbox or @p nullptr
     *        if it's empty.
     */
    template<class Actor>
    unique_mailbox_element_pointer next_message(Actor* self);

    /**
     * @brief Queries whether the mailbox is not empty.
     */
    template<class Actor>
    bool has_next_message(Actor* self);

    void push_to_cache(unique_mailbox_element_pointer ptr);

    typedef std::vector<unique_mailbox_element_pointer> cache_type;

    typedef cache_type::iterator cache_iterator;

    cache_iterator cache_begin();

    cache_iterator cache_end();

    void cache_erase(cache_iterator iter);

};

} // namespace policy
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_PRIORITY_POLICY_HPP
