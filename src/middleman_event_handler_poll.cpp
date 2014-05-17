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


#include "boost/actor/logging.hpp"

#if !defined(BOOST_ACTOR_LINUX) || defined(BOOST_ACTOR_POLL_IMPL)

#ifndef BOOST_ACTOR_WINDOWS
#include <poll.h>
#define SOCKERR errno
#else
#   include <ws2tcpip.h>
#   include <ws2ipdef.h>
#   include <winsock2.h>
#define SOCKERR GetLastError()
#undef EINTR
#undef ENOMEM
#define EINTR WSAEINTR
#define ENOMEM WSAENOBUFS
#endif


#include "boost/actor/io/middleman_event_handler.hpp"

#ifndef POLLRDHUP
#define POLLRDHUP POLLHUP
#endif

namespace boost {
namespace actor {
namespace io {

namespace {

static constexpr unsigned input_event  = POLLIN | POLLPRI;
static constexpr unsigned error_event  = POLLRDHUP | POLLERR | POLLHUP | POLLNVAL;
static constexpr unsigned output_event = POLLOUT;

bool pollfd_less(const pollfd& lhs, native_socket_type rhs) {
    return lhs.fd < rhs;
}

short to_poll_bitmask(event_bitmask mask) {
    switch (mask) {
        case event::read:  return POLLIN;
        case event::write: return POLLOUT;
        case event::both:  return (POLLIN|POLLOUT);
        default: BOOST_ACTOR_CRITICAL("invalid event bitmask");
    }
}

class middleman_event_handler_impl : public middleman_event_handler {

 public:

    void init() { }

 protected:

    void poll_impl() {
        BOOST_ACTOR_REQUIRE(m_pollset.empty() == false);
        BOOST_ACTOR_REQUIRE(m_pollset.size() == m_meta.size());
        int presult = -1;
        while (presult < 0) {
#ifdef BOOST_ACTOR_WINDOWS
            presult = ::WSAPoll(m_pollset.data(), m_pollset.size(), -1);
#else
            presult = ::poll(m_pollset.data(),
                             static_cast<nfds_t>(m_pollset.size()),
                             -1);
#endif
            BOOST_ACTOR_LOG_DEBUG("poll() on " << num_sockets()
                           << " sockets returned " << presult);
            if (presult < 0) {
                switch (SOCKERR) {
                    case EINTR: {
                        // a signal was caught
                        // just try again
                        break;
                    }
                    case ENOMEM: {
                        BOOST_ACTOR_LOG_ERROR("poll() failed for reason ENOMEM");
                        // there's not much we can do other than try again
                        // in hope someone else releases memory
                        break;
                    }
                    default: {
                        perror("poll() failed");
                        BOOST_ACTOR_CRITICAL("poll() failed");
                    }
                }
            }
        }
        for (size_t i = 0; i < m_pollset.size(); ++i) {
            auto mask = static_cast<unsigned>(m_pollset[i].revents);
            auto eb = from_int_bitmask<input_event,
                                       output_event,
                                       error_event>(mask);
            m_pollset[i].revents = 0;
            if (eb != event::none) m_events.emplace_back(eb, m_meta[i].ptr);
        }
    }

    void handle_event(fd_meta_event me,
                      native_socket_type fd,
                      event_bitmask,
                      event_bitmask new_bitmask,
                      continuable*) {
        auto last = m_pollset.end();
        auto iter = std::lower_bound(m_pollset.begin(), last, fd, pollfd_less);
        switch (me) {
            case fd_meta_event::add: {
                pollfd tmp;
                tmp.fd = fd;
                tmp.events = to_poll_bitmask(new_bitmask);
                tmp.revents = 0;
                m_pollset.insert(iter, tmp);
                BOOST_ACTOR_LOG_DEBUG("inserted new element");
                break;
            }
            case fd_meta_event::erase: {
                BOOST_ACTOR_LOG_ERROR_IF(iter == last || iter->fd != fd,
                                  "m_meta and m_pollset out of sync; "
                                  "no element found for fd (cannot erase)");
                if (iter != last && iter->fd == fd) {
                    BOOST_ACTOR_LOG_DEBUG("erased element");
                    m_pollset.erase(iter);
                }
                break;
            }
            case fd_meta_event::mod: {
                BOOST_ACTOR_LOG_ERROR_IF(iter == last || iter->fd != fd,
                                  "m_meta and m_pollset out of sync; "
                                  "no element found for fd (cannot erase)");
                if (iter != last && iter->fd == fd) {
                    BOOST_ACTOR_LOG_DEBUG("updated bitmask");
                    iter->events = to_poll_bitmask(new_bitmask);
                }
                break;
            }
        }
    }

 private:

    std::vector<pollfd> m_pollset; // always in sync with m_meta

};

} // namespace <anonymous>

std::unique_ptr<middleman_event_handler> middleman_event_handler::create() {
    return std::unique_ptr<middleman_event_handler>{new middleman_event_handler_impl};
}

} } // namespace actor
} // namespace boost::io

#else // !defined(BOOST_ACTOR_LINUX) || defined(BOOST_ACTOR_POLL_IMPL)

int keep_compiler_happy_for_poll_impl() { return 42; }

#endif // !defined(BOOST_ACTOR_LINUX) || defined(BOOST_ACTOR_POLL_IMPL)
