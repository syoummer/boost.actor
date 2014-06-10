cmake_minimum_required(VERSION 2.8)
project(boost_actor_io_lib C CXX)

# list hpp files (for convenience when using an IDE)
file(GLOB_RECURSE LIBBOOST_ACTOR_IO_HDRS "${CMAKE_SOURCE_DIR}/boost/actor_io/" *.hpp)

# list cpp files
file(GLOB_RECURSE LIBBOOST_ACTOR_IO_SRCS ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

message(STATUS "boost actor library: ${BOOST_ACTOR_LIBRARY}")

# build shared library if not compiling static only
add_library(libboost_actor_io SHARED ${LIBBOOST_ACTOR_IO_HDRS} ${LIBBOOST_ACTOR_IO_SRCS})
target_link_libraries(libboost_actor_io ${LD_FLAGS} ${BOOST_ACTOR_LIBRARY})
set(LIBRARY_VERSION ${LIBBOOST_ACTOR_IO_VERSION_MAJOR}.${LIBBOOST_ACTOR_IO_VERSION_MINOR}.${LIBBOOST_ACTOR_IO_VERSION_PATCH})
set(LIBRARY_SOVERSION ${LIBBOOST_ACTOR_IO_VERSION_MAJOR})
set_target_properties(libboost_actor_io
                PROPERTIES
                SOVERSION 0
                VERSION 0
                OUTPUT_NAME boost_actor_io)
if(NOT MINGW)
  install(TARGETS libboost_actor_io LIBRARY DESTINATION lib)
endif()

link_directories(${LD_DIRS})
include_directories(${INCLUDE_DIRS})

# install includes
install(DIRECTORY boost/actor/ DESTINATION include/boost/actor FILES_MATCHING PATTERN "*.hpp")

# setting path to boost_actor headers and libboost_actor_io
set (BOOST_ACTOR_IO_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/boost.actor)
set (BOOST_ACTOR_IO_INCLUDE ${BOOST_ACTOR_INCLUDE_PATH})

if ("${BOOST_ACTOR_BUILD_STATIC_ONLY}" STREQUAL "yes")
  set (BOOST_ACTOR_IO_LIBRARY ${LIBRARY_OUTPUT_PATH}/libboost_actor_io_static.a ${LD_FLAGS})
else ()
  if (APPLE)
    if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
      set (BOOST_ACTOR_IO_LIBRARY "-lboost_actor")
    else ()
      set (BOOST_ACTOR_IO_LIBRARY ${LIBRARY_OUTPUT_PATH}/libboost_actor_io.dylib)
    endif ()
    #set (BOOST_ACTOR_IO_LIBRARY -L${LIBRARY_OUTPUT_PATH} -lboost_actor)
  elseif (UNIX)
    set (BOOST_ACTOR_IO_LIBRARY ${LIBRARY_OUTPUT_PATH}/libboost_actor_io.so)
  elseif (MINGW)
    set (BOOST_ACTOR_IO_LIBRARY ${LIBRARY_OUTPUT_PATH}/libboost_actor_io.dll)
  else ()
    message (SEND_FATAL "Host platform not supported ...")
  endif ()
endif ()
