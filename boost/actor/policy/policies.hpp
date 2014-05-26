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


#ifndef BOOST_ACTOR_POLICY_POLICIES_HPP
#define BOOST_ACTOR_POLICY_POLICIES_HPP

namespace boost {
namespace actor {
namespace policy {

template<class SchedulingPolicy, class PriorityPolicy,
         class ResumePolicy, class InvokePolicy>
class policies {

 public:

    typedef SchedulingPolicy scheduling_policy;
    typedef PriorityPolicy   priority_policy;
    typedef ResumePolicy     resume_policy;
    typedef InvokePolicy     invoke_policy;

    scheduling_policy& get_scheduling_policy() {
        return m_scheduling_policy;
    }

    priority_policy& get_priority_policy() {
        return m_priority_policy;
    }

    resume_policy& get_resume_policy() {
        return m_resume_policy;
    }

    invoke_policy& get_invoke_policy() {
        return m_invoke_policy;
    }

 private:

    scheduling_policy m_scheduling_policy;
    priority_policy   m_priority_policy;
    resume_policy     m_resume_policy;
    invoke_policy     m_invoke_policy;

};

} // namespace policy
} // namespace actor
} // namespace boost

#endif // BOOST_ACTOR_POLICY_POLICIES_HPP
