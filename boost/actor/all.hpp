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


#ifndef BOOST_ACTOR_ALL_HPP
#define BOOST_ACTOR_ALL_HPP

#include "boost/actor/on.hpp"
#include "boost/actor/atom.hpp"
#include "boost/actor/send.hpp"
#include "boost/actor/unit.hpp"
#include "boost/actor/actor.hpp"
#include "boost/actor/group.hpp"
#include "boost/actor/spawn.hpp"
#include "boost/actor/config.hpp"
#include "boost/actor/extend.hpp"
#include "boost/actor/policy.hpp"
#include "boost/actor/channel.hpp"
#include "boost/actor/message.hpp"
#include "boost/actor/node_id.hpp"
#include "boost/actor/publish.hpp"
#include "boost/actor/announce.hpp"
#include "boost/actor/anything.hpp"
#include "boost/actor/behavior.hpp"
#include "boost/actor/duration.hpp"
#include "boost/actor/sb_actor.hpp"
#include "boost/actor/shutdown.hpp"
#include "boost/actor/exception.hpp"
#include "boost/actor/resumable.hpp"
#include "boost/actor/scheduler.hpp"
#include "boost/actor/spawn_fwd.hpp"
#include "boost/actor/to_string.hpp"
#include "boost/actor/actor_addr.hpp"
#include "boost/actor/attachable.hpp"
#include "boost/actor/match_expr.hpp"
#include "boost/actor/message_id.hpp"
#include "boost/actor/replies_to.hpp"
#include "boost/actor/serializer.hpp"
#include "boost/actor/actor_proxy.hpp"
#include "boost/actor/exit_reason.hpp"
#include "boost/actor/from_string.hpp"
#include "boost/actor/local_actor.hpp"
#include "boost/actor/ref_counted.hpp"
#include "boost/actor/typed_actor.hpp"
#include "boost/actor/deserializer.hpp"
#include "boost/actor/scoped_actor.hpp"
#include "boost/actor/skip_message.hpp"
#include "boost/actor/actor_ostream.hpp"
#include "boost/actor/spawn_options.hpp"
#include "boost/actor/abstract_actor.hpp"
#include "boost/actor/abstract_group.hpp"
#include "boost/actor/blocking_actor.hpp"
#include "boost/actor/execution_unit.hpp"
#include "boost/actor/memory_managed.hpp"
#include "boost/actor/message_header.hpp"
#include "boost/actor/typed_behavior.hpp"
#include "boost/actor/actor_namespace.hpp"
#include "boost/actor/behavior_policy.hpp"
#include "boost/actor/continue_helper.hpp"
#include "boost/actor/mailbox_element.hpp"
#include "boost/actor/message_builder.hpp"
#include "boost/actor/message_handler.hpp"
#include "boost/actor/response_handle.hpp"
#include "boost/actor/system_messages.hpp"
#include "boost/actor/abstract_channel.hpp"
#include "boost/actor/may_have_timeout.hpp"
#include "boost/actor/message_priority.hpp"
#include "boost/actor/response_promise.hpp"
#include "boost/actor/binary_serializer.hpp"
#include "boost/actor/event_based_actor.hpp"
#include "boost/actor/primitive_variant.hpp"
#include "boost/actor/type_lookup_table.hpp"
#include "boost/actor/uniform_type_info.hpp"
#include "boost/actor/wildcard_position.hpp"
#include "boost/actor/timeout_definition.hpp"
#include "boost/actor/binary_deserializer.hpp"
#include "boost/actor/await_all_actors_done.hpp"
#include "boost/actor/typed_continue_helper.hpp"
#include "boost/actor/typed_event_based_actor.hpp"

/**
 * @author Dominik Charousset <dominik.charousset (at) haw-hamburg.de>
 *
 * @mainpage Boost.Actor
 *
 * @section Intro Introduction
 *
 * This library provides an implementation of the actor model for C++.
 * It uses a network transparent messaging system to ease development
 * of both concurrent and distributed software.
 *
 * @p Boost.Actor uses a thread pool to schedule actors by default.
 * A scheduled actor should not call blocking functions.
 * Individual actors can be spawned (created) with a special flag to run in
 * an own thread if one needs to make use of blocking APIs.
 *
 * Writing applications in @p Boost.Actor requires a minimum of gluecode and
 * each context <i>is</i> an actor. Even main is implicitly
 * converted to an actor if needed.
 *
 * @section GettingStarted Getting Started
 *
 * To build @p Boost.Actor, you need <tt>GCC >= 4.7</tt> or <tt>Clang >= 3.2</tt>,
 * and @p CMake.
 *
 * The usual build steps on Linux and Mac OS X are:
 *
 *- <tt>mkdir build</tt>
 *- <tt>cd build</tt>
 *- <tt>cmake ..</tt>
 *- <tt>make</tt>
 *- <tt>make install</tt> (as root, optionally)
 *
 * Please run the unit tests as well to verify that @p Boost.Actor
 * works properly.
 *
 *- <tt>./bin/unit_tests</tt>
 *
 * Please submit a bug report that includes (a) your compiler version,
 * (b) your OS, and (c) the output of the unit tests if an error occurs.
 *
 * Windows is not supported yet, because MVSC++ doesn't implement the
 * C++11 features needed to compile @p Boost.Actor.
 *
 * Please read the <b>Manual</b> for an introduction to @p Boost.Actor.
 * It is available online as HTML at
 * http://neverlord.github.com/boost.actor/manual/index.html or as PDF at
 * http://neverlord.github.com/boost.actor/manual/manual.pdf
 *
 * @section IntroHelloWorld Hello World Example
 *
 * @include hello_world.cpp
 *
 * @section IntroMoreExamples More Examples
 *
 * The {@link math_actor.cpp Math Actor Example} shows the usage
 * of {@link receive_loop} and {@link cppa::arg_match arg_match}.
 * The {@link dining_philosophers.cpp Dining Philosophers Example}
 * introduces event-based actors and includes a lot of <tt>Boost.Actor</tt>
 * features.
 *
 * @namespace cppa
 * @brief Root namespace of Boost.Actor.
 *
 * @namespace cppa::util
 * @brief Contains utility classes and metaprogramming
 *        utilities used by the Boost.Actor implementation.
 *
 * @namespace cppa::intrusive
 * @brief Contains intrusive container implementations.
 *
 * @namespace cppa::opencl
 * @brief Contains all classes of Boost.Actor's OpenCL binding (optional).
 *
 * @namespace cppa::network
 * @brief Contains all network related classes.
 *
 * @namespace cppa::factory
 * @brief Contains factory functions to create actors from lambdas or
 *        other functors.
 *
 * @namespace cppa::exit_reason
 * @brief Contains all predefined exit reasons.
 *
 * @namespace cppa::placeholders
 * @brief Contains the guard placeholders @p _x1 to @p _x9.
 *
 * @defgroup CopyOnWrite Copy-on-write optimization.
 * @p Boost.Actor uses a copy-on-write optimization for its message
 * passing implementation.
 *
 * {@link cppa::cow_tuple Tuples} should @b always be used with by-value
 * semantic, since tuples use a copy-on-write smart pointer internally.
 * Let's assume two
 * tuple @p x and @p y, whereas @p y is a copy of @p x:
 *
 * @code
 * auto x = make_message(1, 2, 3);
 * auto y = x;
 * @endcode
 *
 * Those two tuples initially point to the same data (the addresses of the
 * first element of @p x is equal to the address of the first element
 * of @p y):
 *
 * @code
 * assert(&(get<0>(x)) == &(get<0>(y)));
 * @endcode
 *
 * <tt>get<0>(x)</tt> returns a const-reference to the first element of @p x.
 * The function @p get does not have a const-overload to avoid
 * unintended copies. The function @p get_ref could be used to
 * modify tuple elements. A call to this function detaches
 * the tuple by copying the data before modifying it if there are two or more
 * references to the data:
 *
 * @code
 * // detaches x from y
 * get_ref<0>(x) = 42;
 * // x and y no longer point to the same data
 * assert(&(get<0>(x)) != &(get<0>(y)));
 * @endcode
 *
 * @defgroup MessageHandling Message handling.
 *
 * @brief This is the beating heart of @p Boost.Actor. Actor programming is
 *        all about message handling.
 *
 * A message in @p Boost.Actor is a n-tuple of values (with size >= 1)
 * You can use almost every type in a messages - as long as it is announced,
 * i.e., known by the type system of @p Boost.Actor.
 *
 * @defgroup BlockingAPI Blocking API.
 *
 * @brief Blocking functions to receive messages.
 *
 * The blocking API of Boost.Actor is intended to be used for migrating
 * previously threaded applications. When writing new code, you should use
 * ibcppas nonblocking become/unbecome API.
 *
 * @section Send Send messages
 *
 * The function @p send can be used to send a message to an actor.
 * The first argument is the receiver of the message followed by any number
 * of values:
 *
 * @code
 * // spawn some actors
 * auto a1 = spawn(...);
 * auto a2 = spawn(...);
 * auto a3 = spawn(...);
 *
 * // send a message to a1
 * send(a1, atom("hello"), "hello a1!");
 *
 * // send a message to a1, a2, and a3
 * auto msg = make_message(atom("compute"), 1, 2, 3);
 * send(a1, msg);
 * send(a2, msg);
 * send(a3, msg);
 * @endcode
 *
 * @section Receive Receive messages
 *
 * The function @p receive takes a @p behavior as argument. The behavior
 * is a list of { pattern >> callback } rules.
 *
 * @code
 * receive
 * (
 *     on(atom("hello"), arg_match) >> [](const std::string& msg)
 *     {
 *         cout << "received hello message: " << msg << endl;
 *     },
 *     on(atom("compute"), arg_match) >> [](int i0, int i1, int i2)
 *     {
 *         // send our result back to the sender of this messages
 *         return make_message(atom("result"), i0 + i1 + i2);
 *     }
 * );
 * @endcode
 *
 * Please read the manual for further details about pattern matching.
 *
 * @section Atoms Atoms
 *
 * Atoms are a nice way to add semantic informations to a message.
 * Assuming an actor wants to provide a "math sevice" for integers. It
 * could provide operations such as addition, subtraction, etc.
 * This operations all have two operands. Thus, the actor does not know
 * what operation the sender of a message wanted by receiving just two integers.
 *
 * Example actor:
 * @code
 * void math_actor() {
 *     receive_loop (
 *         on(atom("plus"), arg_match) >> [](int a, int b) {
 *             return make_message(atom("result"), a + b);
 *         },
 *         on(atom("minus"), arg_match) >> [](int a, int b) {
 *             return make_message(atom("result"), a - b);
 *         }
 *     );
 * }
 * @endcode
 *
 * @section ReceiveLoops Receive loops
 *
 * Previous examples using @p receive create behaviors on-the-fly.
 * This is inefficient in a loop since the argument passed to receive
 * is created in each iteration again. It's possible to store the behavior
 * in a variable and pass that variable to receive. This fixes the issue
 * of re-creation each iteration but rips apart definition and usage.
 *
 * There are four convenience functions implementing receive loops to
 * declare behavior where it belongs without unnecessary
 * copies: @p receive_loop, @p receive_while, @p receive_for and @p do_receive.
 *
 * @p receive_loop is analogous to @p receive and loops "forever" (until the
 * actor finishes execution).
 *
 * @p receive_while creates a functor evaluating a lambda expression.
 * The loop continues until the given lambda returns @p false. A simple example:
 *
 * @code
 * // receive two integers
 * vector<int> received_values;
 * receive_while([&]() { return received_values.size() < 2; }) (
 *     on<int>() >> [](int value) {
 *         received_values.push_back(value);
 *     }
 * );
 * // ...
 * @endcode
 *
 * @p receive_for is a simple ranged-based loop:
 *
 * @code
 * std::vector<int> vec {1, 2, 3, 4};
 * auto i = vec.begin();
 * receive_for(i, vec.end()) (
 *     on(atom("get")) >> [&]() -> message { return {atom("result"), *i}; }
 * );
 * @endcode
 *
 * @p do_receive returns a functor providing the function @p until that
 * takes a lambda expression. The loop continues until the given lambda
 * returns true. Example:
 *
 * @code
 * // receive ints until zero was received
 * vector<int> received_values;
 * do_receive (
 *     on<int>() >> [](int value) {
 *         received_values.push_back(value);
 *     }
 * )
 * .until([&]() { return received_values.back() == 0 });
 * // ...
 * @endcode
 *
 * @section FutureSend Send delayed messages
 *
 * The function @p delayed_send provides a simple way to delay a message.
 * This is particularly useful for recurring events, e.g., periodical polling.
 * Usage example:
 *
 * @code
 * delayed_send(self, std::chrono::seconds(1), atom("poll"));
 * receive_loop (
 *     // ...
 *     on(atom("poll")) >> [] {
 *         // ... poll something ...
 *         // and do it again after 1sec
 *         delayed_send(self, std::chrono::seconds(1), atom("poll"));
 *     }
 * );
 * @endcode
 *
 * See also the {@link dancing_kirby.cpp dancing kirby example}.
 *
 * @defgroup ImplicitConversion Implicit type conversions.
 *
 * The message passing of @p Boost.Actor prohibits pointers in messages because
 * it enforces network transparent messaging.
 * Unfortunately, string literals in @p C++ have the type <tt>const char*</tt>,
 * resp. <tt>const char[]</tt>. Since @p Boost.Actor is a user-friendly library,
 * it silently converts string literals and C-strings to @p std::string objects.
 * It also converts unicode literals to the corresponding STL container.
 *
 * A few examples:
 * @code
 * // sends an std::string containing "hello actor!" to itself
 * send(self, "hello actor!");
 *
 * const char* cstring = "cstring";
 * // sends an std::string containing "cstring" to itself
 * send(self, cstring);
 *
 * // sends an std::u16string containing the UTF16 string "hello unicode world!"
 * send(self, u"hello unicode world!");
 *
 * // x has the type cppa::tuple<std::string, std::string>
 * auto x = make_message("hello", "tuple");
 *
 * receive (
 *     // equal to: on(std::string("hello actor!"))
 *     on("hello actor!") >> [] { }
 * );
 * @endcode
 *
 * @defgroup ActorCreation Actor creation.
 *
 * @defgroup MetaProgramming Metaprogramming utility.
 */

// examples

/**
 * @brief A trivial example program.
 * @example hello_world.cpp
 */

/**
 * @brief Shows the usage of {@link cppa::atom atoms}
 *        and {@link cppa::arg_match arg_match}.
 * @example math_actor.cpp
 */

/**
 * @brief A simple example for a delayed_send based application.
 * @example dancing_kirby.cpp
 */

/**
 * @brief An event-based "Dining Philosophers" implementation.
 * @example dining_philosophers.cpp
 */

#endif // BOOST_ACTOR_ALL_HPP
