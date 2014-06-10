cmake_minimum_required(VERSION 2.8)
project(boost_actor_lib C CXX)

# list cpp files
file(GLOB_RECURSE LIBBOOST_ACTOR_SRCS ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# build shared library if not compiling static only
add_library(libboost_actor SHARED ${LIBBOOST_ACTOR_HDRS} ${LIBBOOST_ACTOR_SRCS})
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
