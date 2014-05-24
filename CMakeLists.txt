cmake_minimum_required(VERSION 2.8)
project(boost_actor C CXX)

# prohibit in-source builds
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please use "
                        "./configure to choose a build directory and "
                        "initialize the build configuration.")
endif ()

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# set binary output path
if ("${EXECUTABLE_OUTPUT_PATH}" STREQUAL "")
  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
endif ()

# set library output path
if ("${LIBRARY_OUTPUT_PATH}" STREQUAL "")
  set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
endif ()

# set library output path to binary output path for Xcode projects
if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
  set(LIBRARY_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}")
endif ()

# check for g++ >= 4.7 or clang++ > = 3.2
try_run(ProgramResult
        CompilationSucceeded
  ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/get_compiler_version.cpp
        RUN_OUTPUT_VARIABLE CompilerVersion)
if (NOT CompilationSucceeded OR NOT ProgramResult EQUAL 0)
  message(FATAL_ERROR "Cannot determine compiler version")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if (CompilerVersion VERSION_GREATER 4.6)
    message(STATUS "Found g++ version ${CompilerVersion}")
  else ()
    message(FATAL_ERROR "g++ >= 4.7 required (found: ${CompilerVersion}.")
  endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if (CompilerVersion VERSION_GREATER 3.1)
    message(STATUS "Found clang++ version ${CompilerVersion}")
  else ()
    message(FATAL_ERROR "clang++ >= 3.2 required (found: ${CompilerVersion}.")
  endif ()
  if (NOT CXXFLAGS_PROVIDED)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
    message(STATUS "NOTE: Automatically added -stdlib=libc++ flag, "
                   "you can override this by defining CMAKE_CXX_FLAGS "
                   "(see 'configure --help')")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif ()
else ()
  message(FATAL_ERROR "Your C++ compiler does not support C++11 "
                      "or is not supported")
endif ()

# check if the user provided CXXFLAGS on the command line
if (CMAKE_CXX_FLAGS)
  set(CXXFLAGS_PROVIDED true)
  set(CMAKE_CXX_FLAGS_DEBUG          "")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "")
  set(CMAKE_CXX_FLAGS_RELEASE        "")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
else (CMAKE_CXX_FLAGS)
  set(CXXFLAGS_PROVIDED false)
  if (MORE_CLANG_WARNINGS AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -Weverything -Wno-c++98-compat -Wno-padded -Wno-documentation-unknown-command -Wno-exit-time-destructors -Wno-global-constructors -Wno-missing-prototypes -Wno-c++98-compat-pedantic -Wno-unused-member-function -Wno-unused-const-variable -ftemplate-backtrace-limit=0")
  else ()
    set(CMAKE_CXX_FLAGS "-std=c++11 -Wextra -Wall -pedantic -ftemplate-backtrace-limit=0")
  endif ()
  if (MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN32")
    include (GenerateExportHeader)
  endif(MINGW)
  set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
endif (CMAKE_CXX_FLAGS)

include(CheckIncludeFileCXX)
check_include_file_cxx("valgrind/valgrind.h" HAVE_VALGRIND_H)
if (HAVE_VALGRIND_H)
  set(VALGRIND "yes")
  add_definitions(-DBOOST_ACTOR_ANNOTATE_VALGRIND)
else (HAVE_VALGRIND_H)
  set(VALGRIND "no")
endif (HAVE_VALGRIND_H)

# set build type (evaluate ENABLE_DEBUG flag)
if (ENABLE_DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
  add_definitions(-DBOOST_ACTOR_DEBUG_MODE)
  message(STATUS "Enable libboost_actor's debug mode")
endif (ENABLE_DEBUG)

if (ENABLE_ADDRESS_SANITIZER)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  message(STATUS "Enable address sanitizer")
endif (ENABLE_ADDRESS_SANITIZER)

if (BOOST_ACTOR_LOG_LEVEL)
  add_definitions(-DBOOST_ACTOR_LOG_LEVEL=${BOOST_ACTOR_LOG_LEVEL})
endif(BOOST_ACTOR_LOG_LEVEL)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# set build default build type if not set
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif ("${CMAKE_BUILD_TYPE}" STREQUAL "")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LIBBOOST_ACTOR_PLATFORM_SRC src/middleman_event_handler_epoll.cpp)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(LIBBOOST_ACTOR_PLATFORM_SRC src/middleman_event_handler_poll.cpp)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Window")
  set(LIBBOOST_ACTOR_PLATFORM_SRC src/middleman_event_handler_poll.cpp src/execinfo_windows.cpp)
else ()
  message(FATAL_ERROR "This platform is not supported by libboost_actor")
endif()

# list cpp files including platform-dependent files
set(LIBBOOST_ACTOR_SRC
    ${LIBBOOST_ACTOR_PLATFORM_SRC}
    src/abstract_actor.cpp
    src/abstract_channel.cpp
    src/abstract_group.cpp
    src/abstract_tuple.cpp
    src/acceptor.cpp
    src/actor.cpp
    src/actor_addr.cpp
    src/actor_namespace.cpp
    src/actor_proxy.cpp
    src/actor_registry.cpp
    src/actor_ostream.cpp
    src/atom.cpp
    src/attachable.cpp
    src/behavior.cpp
    src/behavior_stack.cpp
    src/binary_deserializer.cpp
    src/binary_serializer.cpp
    src/broker.cpp
    src/buffer.cpp
    src/blocking_actor.cpp
    src/channel.cpp
    src/context_switching_resume.cpp
    src/continuable.cpp
    src/continue_helper.cpp
    src/cs_thread.cpp
    src/decorated_tuple.cpp
    src/default_message_queue.cpp
    src/actor_proxy.cpp
    src/peer.cpp
    src/peer_acceptor.cpp
    src/demangle.cpp
    src/deserializer.cpp
    src/duration.cpp
    src/event_based_actor.cpp
    src/exception.cpp
    src/execution_unit.cpp
    src/exit_reason.cpp
    src/fd_util.cpp
    src/functor_based_actor.cpp
    src/functor_based_blocking_actor.cpp
    src/get_root_uuid.cpp
    src/get_mac_addresses.cpp
    src/group.cpp
    src/group_manager.cpp
    src/input_stream.cpp
    src/local_actor.cpp
    src/logging.cpp
    src/mailbox_element.cpp
    src/max_msg_size.cpp
    src/memory.cpp
    src/memory_managed.cpp
    src/message.cpp
    src/message_builder.cpp
    src/message_header.cpp
    src/middleman.cpp
    src/middleman_event_handler.cpp
    src/node_id.cpp
    src/output_stream.cpp
    src/partial_function.cpp
    src/ref_counted.cpp
    src/resumable.cpp
    src/remote_actor_proxy.cpp
    src/response_promise.cpp
    src/ripemd_160.cpp
    src/scheduler.cpp
    src/scoped_actor.cpp
    src/serializer.cpp
    src/shared_spinlock.cpp
    src/singleton_manager.cpp
    src/stream.cpp
    src/string_serialization.cpp
    src/sync_request_bouncer.cpp
    src/tcp_acceptor.cpp
    src/tcp_io_stream.cpp
    src/to_uniform_name.cpp
    src/type_lookup_table.cpp
    src/unicast_network.cpp
    src/uniform_type_info.cpp
    src/uniform_type_info_map.cpp
    src/yield_interface.cpp)

if (BOOST_ROOT)
  # Prevent falling back to system paths when using a custom Boost prefix.
  set(Boost_NO_SYSTEM_PATHS true)
endif ()

set(INCLUDE_DIRS .)
set(LD_DIRS)
set(LD_FLAGS ${CMAKE_LD_LIBS})

if (APPLE)
  # -pthread is ignored on MacOSX
elseif (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
elseif (MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
  set(LD_FLAGS "ws2_32 -liphlpapi")
endif ()

if (DISABLE_MEM_MANAGEMENT)
  add_definitions(-DBOOST_ACTOR_DISABLE_MEM_MANAGEMENT)
endif (DISABLE_MEM_MANAGEMENT)

if (STANDALONE_BUILD)
  add_definitions(-DBOOST_ACTOR_STANDALONE_BUILD)
endif (STANDALONE_BUILD)

#find_package(Boost 1.54.0 COMPONENTS REQUIRED system context coroutine thread)
find_package(Boost 1.53.0 COMPONENTS REQUIRED system thread)
set(LD_FLAGS ${LD_FLAGS} ${Boost_COROUTINE_LIBRARY} ${Boost_CONTEXT_LIBRARY} ${Boost_SYSTEM_LIBRARY})
# This hack fixes a problem when the linker search path is the same as
# the one provided by the compiler. In this case, CMake replaces the absolute
# path (e.g., /path/to/lib.so) with -l<lib>, which may cause it to pick up the wrong
# library. So when this replacement happens, we ensure that the right
# library gets picked by adding a -L directive for the affected libraries
# (which is just Boost Context here).
set(CMAKE_EXE_LINKER_FLAGS -L${Boost_LIBRARY_DIRS})
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
set(LD_DIRS ${LD_DIRS} ${Boost_LIBRARY_DIRS})

# set compiler flag when compiling w/o context-switching actors
# DEFAULT for now, because the Context API seems to be broken again
#if (DISABLE_CONTEXT_SWITCHING)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_ACTOR_DISABLE_CONTEXT_SWITCHING")
#endif ()

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

link_directories(${LD_DIRS})
include_directories(${INCLUDE_DIRS})

# install includes
install(DIRECTORY boost/actor/ DESTINATION include/boost/actor FILES_MATCHING PATTERN "*.hpp")

# process cmake_uninstall.cmake.in
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)

# add uninstall target
add_custom_target(uninstall
                  COMMAND ${CMAKE_COMMAND} -P
                  ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# setting path to boost_actor headers and libboost_actor
set (BOOST_ACTOR_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/boost.actor)
set (BOOST_ACTOR_INCLUDE ${BOOST_ACTOR_INCLUDE_PATH})

if ("${BOOST_ACTOR_BUILD_STATIC_ONLY}" STREQUAL "yes")
  set (BOOST_ACTOR_LIBRARY ${LIBRARY_OUTPUT_PATH}/libboost_actor_static.a ${LD_FLAGS})
else ()
  if (APPLE)
    if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
      set (BOOST_ACTOR_LIBRARY "-lboost_actor")
    else ()
      set (BOOST_ACTOR_LIBRARY ${LIBRARY_OUTPUT_PATH}/libboost_actor.dylib)
    endif ()
    #set (BOOST_ACTOR_LIBRARY -L${LIBRARY_OUTPUT_PATH} -lboost_actor)
  elseif (UNIX)
    set (BOOST_ACTOR_LIBRARY ${LIBRARY_OUTPUT_PATH}/libboost_actor.so)
  elseif (MINGW)
    set (BOOST_ACTOR_LIBRARY ${LIBRARY_OUTPUT_PATH}/libboost_actor.dll)
  else ()
    message (SEND_FATAL "Host platform not supported ...")
  endif ()
endif ()

# set up subdirectories
if (NOT "${BOOST_ACTOR_NO_UNIT_TESTS}" STREQUAL "yes")
  enable_testing()
  add_subdirectory(unit_testing)
  if ("${BOOST_ACTOR_BUILD_STATIC_ONLY}" STREQUAL "yes")
    add_dependencies(all_unit_tests libboost_actorStatic)
  else ()
    add_dependencies(all_unit_tests libboost_actor)
  endif ()
endif ()
if (NOT "${BOOST_ACTOR_NO_EXAMPLES}" STREQUAL "yes")
  add_subdirectory(examples)
  if ("${BOOST_ACTOR_BUILD_STATIC_ONLY}" STREQUAL "yes")
    add_dependencies(all_examples libboost_actorStatic)
  else ()
    add_dependencies(all_examples libboost_actor)
  endif ()
endif ()

# set optional flags
string(TOUPPER ${CMAKE_BUILD_TYPE} build_type)
set(CONTEXT_SWITCHING "yes")
if (DISABLE_CONTEXT_SWITCHING)
    set(CONTEXT_SWITCHING "no")
endif ()

# check for doxygen and add custom "doc" target to Makefile
find_package(Doxygen)
if (DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
                 @ONLY)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_HOME_DIRECTORY}/Doxyfile
                    WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
                    COMMENT "Generating API documentation with Doxygen"
                    VERBATIM)
endif (DOXYGEN_FOUND)

# set variables for summary
set(LOG_LEVEL_STR "none")
if (BOOST_ACTOR_LOG_LEVEL)
  if (${BOOST_ACTOR_LOG_LEVEL} EQUAL 0)
    set(LOG_LEVEL_STR "ERROR")
  elseif (${BOOST_ACTOR_LOG_LEVEL} EQUAL 1)
    set(LOG_LEVEL_STR "WARNING")
  elseif (${BOOST_ACTOR_LOG_LEVEL} EQUAL 2)
    set(LOG_LEVEL_STR "INFO")
  elseif (${BOOST_ACTOR_LOG_LEVEL} EQUAL 3)
    set(LOG_LEVEL_STR "DEBUG")
  elseif (${BOOST_ACTOR_LOG_LEVEL} EQUAL 4)
    set(LOG_LEVEL_STR "TRACE")
  else ()
    set(LOG_LEVEL_STR "invalid")
  endif ()
endif (BOOST_ACTOR_LOG_LEVEL)

macro (toYesNo in out)
  if (${in})
    set(${out} "yes")
  else ()
    set(${out} "no")
  endif ()
endmacro ()

macro (invertYesNo in out)
  if ("${in}" STREQUAL "yes")
    set(${out} "no")
  else ()
    set(${out} "yes")
  endif ()
endmacro ()

toYesNo(ENABLE_DEBUG DEBUG_MODE_STR)
toYesNo(ENABLE_OPENCL BUILD_OPENCL_STR)
toYesNo(DISABLE_MEM_MANAGEMENT DISABLE_MEM_MANAGEMENT_STR)
invertYesNo(BOOST_ACTOR_NO_EXAMPLES BUILD_EXAMPLES)
invertYesNo(BOOST_ACTOR_NO_UNIT_TESTS BUILD_UNIT_TESTS)
invertYesNo(DISABLE_MEM_MANAGEMENT_STR WITH_MEM_MANAGEMENT)

if (NOT "${BOOST_ACTOR_BUILD_STATIC}" STREQUAL "yes")
  set(BOOST_ACTOR_BUILD_STATIC "no")
endif ()

if (NOT "${BOOST_ACTOR_BUILD_STATIC_ONLY}" STREQUAL "yes")
  set(BOOST_ACTOR_BUILD_STATIC_ONLY "no")
else ()
  set(BOOST_ACTOR_BUILD_STATIC "yes")
endif ()

# done (print summary)
message("\n====================|  Build Summary  |===================="
        "\n"
        "\nBuild type:        ${CMAKE_BUILD_TYPE}"
        "\nDebug mode:        ${DEBUG_MODE_STR}"
        "\nLog level:         ${LOG_LEVEL_STR}"
        "\nContext switching: ${CONTEXT_SWITCHING}"
        "\nValgrind:          ${VALGRIND}"
        "\nBuild examples:    ${BUILD_EXAMPLES}"
        "\nBuild unit tests:  ${BUILD_UNIT_TESTS}"
        "\nBuild static:      ${BOOST_ACTOR_BUILD_STATIC}"
        "\nBulid static only: ${BOOST_ACTOR_BUILD_STATIC_ONLY}"
        "\nBuild OpenCL:      ${BUILD_OPENCL_STR}"
        "\nWith mem. mgmt.:   ${WITH_MEM_MANAGEMENT}"
        "\n"
        "\nCXX:               ${CMAKE_CXX_COMPILER}"
        "\nCXXFLAGS:          ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${build_type}}"
        "\nLD_DIRS:           ${LD_DIRS}"
        "\nLIBRARIES:         ${LD_FLAGS}"
        "\n"
        "\nSource directory:  ${CMAKE_SOURCE_DIR}"
        "\nBuild directory:   ${CMAKE_BINARY_DIR}"
        "\nExecutable path:   ${EXECUTABLE_OUTPUT_PATH}"
        "\nLibrary path:      ${LIBRARY_OUTPUT_PATH}"
        "\nInstall prefix:    ${CMAKE_INSTALL_PREFIX}"
        "\n"
        "\nBoost:             ${Boost_INCLUDE_DIR}"
        "\n"
        "\n===========================================================\n")
