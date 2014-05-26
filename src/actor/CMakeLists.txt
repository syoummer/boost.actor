cmake_minimum_required(VERSION 2.8)
project(boost_actor_lib C CXX)

# list cpp files including platform-dependent files
set(LIBBOOST_ACTOR_SRC
    abstract_actor.cpp
    abstract_channel.cpp
    abstract_group.cpp
    abstract_tuple.cpp
    actor.cpp
    actor_addr.cpp
    actor_namespace.cpp
    actor_ostream.cpp
    actor_proxy.cpp
    actor_registry.cpp
    atom.cpp
    attachable.cpp
    behavior.cpp
    behavior_stack.cpp
    blocking_actor.cpp
    binary_deserializer.cpp
    channel.cpp
    context_switching_resume.cpp
    continue_helper.cpp
    cs_thread.cpp
    decorated_tuple.cpp
    demangle.cpp
    deserializer.cpp
    duration.cpp
    event_based_actor.cpp
    exception.cpp
    execution_unit.cpp
    exit_reason.cpp
    functor_based_actor.cpp
    functor_based_blocking_actor.cpp
    get_mac_addresses.cpp
    get_root_uuid.cpp
    group.cpp
    group_manager.cpp
    local_actor.cpp
    logging.cpp
    mailbox_element.cpp
    max_msg_size.cpp
    memory.cpp
    memory_managed.cpp
    message.cpp
    message_builder.cpp
    message_header.cpp
    node_id.cpp
    partial_function.cpp
    ref_counted.cpp
    response_promise.cpp
    resumable.cpp
    ripemd_160.cpp
    scheduler.cpp
    scoped_actor.cpp
    serializer.cpp
    shared_spinlock.cpp
    singletons.cpp
    string_serialization.cpp
    sync_request_bouncer.cpp
    to_uniform_name.cpp
    type_lookup_table.cpp
    uniform_type_info.cpp
    uniform_type_info_map.cpp
    yield_interface.cpp)

# build shared library if not compiling static only
if (NOT "${BOOST_ACTOR_BUILD_STATIC_ONLY}" STREQUAL "yes")
  add_library(libboost_actor SHARED ${LIBBOOST_ACTOR_SRC} ${LIBBOOST_ACTOR_HDRS})
  target_link_libraries(libboost_actor ${LD_FLAGS})
  set(LIBRARY_VERSION ${LIBBOOST_ACTOR_VERSION_MAJOR}.${LIBBOOST_ACTOR_VERSION_MINOR}.${LIBBOOST_ACTOR_VERSION_PATCH})
  set(LIBRARY_SOVERSION ${LIBBOOST_ACTOR_VERSION_MAJOR})
  set_target_properties(libboost_actor
                        PROPERTIES
                        SOVERSION 0
                        VERSION 0
                        OUTPUT_NAME boost_actor)
  if(NOT MINGW)
    install(TARGETS libboost_actor LIBRARY DESTINATION lib)
  endif()
endif ()

# build static library only if --build-static or --build-static-only was set
if ("${BOOST_ACTOR_BUILD_STATIC_ONLY}" STREQUAL "yes" OR "${BOOST_ACTOR_BUILD_STATIC}" STREQUAL "yes")
  add_library(libboost_actorStatic STATIC ${LIBBOOST_ACTOR_HDRS} ${LIBBOOST_ACTOR_SRC})
  set_target_properties(libboost_actorStatic PROPERTIES OUTPUT_NAME boost_actor_static)
  install(TARGETS libboost_actorStatic ARCHIVE DESTINATION lib)
endif ()
