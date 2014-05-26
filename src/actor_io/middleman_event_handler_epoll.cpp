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


#include "boost/actor/config.hpp"

#if defined(BOOST_ACTOR_LINUX) && !defined(BOOST_ACTOR_POLL_IMPL)

#include <ios>
#include <string>
#include <vector>

#include <string.h>
#include <sys/epoll.h>

#include "boost/actor/detail/logging.hpp"
#include "boost/actor_io/middleman_event_handler.hpp"

namespace boost {
namespace actor_io {

namespace {

static constexpr unsigned input_event  = EPOLLIN;
static constexpr unsigned error_event  = EPOLLRDHUP | EPOLLERR | EPOLLHUP;
static constexpr unsigned output_event = EPOLLOUT;

class middleman_event_handler_impl : public middleman_event_handler {

 public:

    middleman_event_handler_impl() : m_epollfd(-1) { }

    ~middleman_event_handler_impl() { if (m_epollfd != -1) close(m_epollfd); }

    void init() {
        m_epollfd = epoll_create1(EPOLL_CLOEXEC);
        if (m_epollfd == -1) {
            throw std::ios_base::failure(  std::string("epoll_create1: ")
                                         + strerror(errno));
        }
        // handle at most 64 events at a time
        m_epollset.resize(64);
    }

 protected:

    void poll_impl() {
        BOOST_ACTOR_REQUIRE(m_meta.empty() == false);
        int presult = -1;
        while (presult < 0) {
            presult = epoll_wait(m_epollfd,
                                 m_epollset.data(),
                                 static_cast<int>(m_epollset.size()),
                                 -1);
            BOOST_ACTOR_LOG_DEBUG("epoll_wait on " << num_sockets()
                           << " sockets returned " << presult);
            if (presult < 0) {
                switch (errno) {
                    case EINTR: {
                        // a signal was caught
                        // just try again
                        break;
                    }
                    default: {
                        perror("epoll_wait() failed");
                        BOOST_ACTOR_CRITICAL("epoll_wait() failed");
                    }
                }
            }
        }
        auto iter = m_epollset.begin();
        auto last = iter + static_cast<size_t>(presult);
        for ( ; iter != last; ++iter) {
            auto eb = from_int_bitmask<input_event,
                                       output_event,
                                       error_event>(iter->events);
            auto ptr = reinterpret_cast<continuable*>(iter->data.ptr);
            BOOST_ACTOR_REQUIRE(eb != event::none);
            m_events.emplace_back(eb, ptr);
        }
    }

    void handle_event(fd_meta_event me,
                      native_socket_type fd,
                      event_bitmask,
                      event_bitmask new_bitmask,
                      continuable* ptr) {
        int operation;
        epoll_event ee;
        ee.data.ptr = ptr;
        switch (new_bitmask) {
            case event::none:
                BOOST_ACTOR_REQUIRE(me == fd_meta_event::erase);
                ee.events = 0;
                break;
            case event::read:
                ee.events = EPOLLIN | EPOLLRDHUP;
                break;
            case event::write:
                ee.events = EPOLLOUT;
            case event::both:
                ee.events = EPOLLIN | EPOLLRDHUP | EPOLLOUT;
                break;
            default: BOOST_ACTOR_CRITICAL("invalid event bitmask");
        }
        switch (me) {
            case fd_meta_event::add:
                operation = EPOLL_CTL_ADD;
                break;
            case fd_meta_event::erase:
                operation = EPOLL_CTL_DEL;
                break;
            case fd_meta_event::mod:
                operation = EPOLL_CTL_MOD;
                break;
            default: BOOST_ACTOR_CRITICAL("invalid fd_meta_event");
        }
        if (epoll_ctl(m_epollfd, operation, fd, &ee) < 0) {
            switch (errno) {
                // supplied file descriptor is already registered
                case EEXIST: {
                    BOOST_ACTOR_LOG_ERROR("file descriptor registered twice");
                    break;
                }
                // op was EPOLL_CTL_MOD or EPOLL_CTL_DEL,
                // and fd is not registered with this epoll instance.
                case ENOENT: {
                    BOOST_ACTOR_LOG_ERROR("cannot delete file descriptor "
                                   "because it isn't registered");
                    break;
                }
                default: {
                    BOOST_ACTOR_LOG_ERROR(strerror(errno));
                    perror("epoll_ctl() failed");
                    BOOST_ACTOR_CRITICAL("epoll_ctl() failed");
                }
            }
        }
    }

 private:

    int m_epollfd;
    std::vector<epoll_event> m_epollset;

};

} // namespace <anonymous>

std::unique_ptr<middleman_event_handler> middleman_event_handler::create() {
    return std::unique_ptr<middleman_event_handler>{new middleman_event_handler_impl};
}

} // namespace actor_io
} // namespace boost

#else // defined(BOOST_ACTOR_LINUX) && !defined(BOOST_ACTOR_POLL_IMPL)

int keep_compiler_happy_for_epoll_impl() { return 42; }

#endif // defined(BOOST_ACTOR_LINUX) && !defined(BOOST_ACTOR_POLL_IMPL)
