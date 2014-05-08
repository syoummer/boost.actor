#ifndef PING_PONG_HPP
#define PING_PONG_HPP

//#include "boost/actor/actor.hpp"

#include <cstddef>
#include "boost/actor/fwd.hpp"

void ping(boost::actor::blocking_actor*, size_t num_pings);

void event_based_ping(boost::actor::event_based_actor*, size_t num_pings);

void pong(boost::actor::blocking_actor*, boost::actor::actor ping_actor);

void event_based_pong(boost::actor::event_based_actor*,
                      boost::actor::actor ping_actor);

// returns the number of messages ping received
size_t pongs();

#endif // PING_PONG_HPP
