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


#include <iterator>

#include "boost/none.hpp"

#include "boost/actor/local_actor.hpp"
#include "boost/actor/detail/behavior_stack.hpp"

using namespace std;

namespace boost {
namespace actor {
namespace detail {

struct behavior_stack_mover : iterator<output_iterator_tag, void, void, void, void>{

 public:

    behavior_stack_mover(behavior_stack* st) : m_stack(st) { }

    behavior_stack_mover& operator=(behavior_stack::element_type&& rval) {
        m_stack->m_erased_elements.emplace_back(move(rval.first));
        return *this;
    }

    behavior_stack_mover& operator*() { return *this; }

    behavior_stack_mover& operator++() { return *this; }

    behavior_stack_mover operator++(int) { return *this; }

 private:

    behavior_stack* m_stack;

};

inline behavior_stack_mover move_iter(behavior_stack* bs) { return {bs}; }

optional<behavior&> behavior_stack::sync_handler(message_id expected_response) {
    if (expected_response.valid()) {
        auto e = m_elements.rend();
        auto i = find_if(m_elements.rbegin(), e, [=](element_type& val) {
            return val.second == expected_response;
        });
        if (i != e) return i->first;
    }
    return none;
}

void behavior_stack::pop_async_back() {
    if (m_elements.empty()) { } // nothing to do
    else if (!m_elements.back().second.valid()) erase_at(m_elements.end() - 1);
    else rerase_if([](const element_type& e) {
        return e.second.valid() == false;
    });
}

void behavior_stack::clear() {
    if (m_elements.empty() == false) {
        std::move(m_elements.begin(), m_elements.end(), move_iter(this));
        m_elements.clear();
    }
}

} } // namespace actor
} // namespace boost::detail
