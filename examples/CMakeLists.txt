cmake_minimum_required(VERSION 2.8)
project(cppa_examples CXX)

find_package(Boost 1.54.0 COMPONENTS REQUIRED program_options)
set(LD_FLAGS ${LD_FLAGS} ${Boost_PROGRAM_OPTIONS_LIBRARY})
# This hack fixes a problem when the linker search path is the same as
# the one provided by the compiler. In this case, CMake replaces the absolute
# path (e.g., /path/to/lib.so) with -l<lib>, which may cause it to pick up the wrong
# library. So when this replacement happens, we ensure that the right
# library gets picked by adding a -L directive for the affected libraries
# (which is just Boost Context here).
set(CMAKE_EXE_LINKER_FLAGS -L${Boost_LIBRARY_DIRS})
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
set(LD_DIRS ${LD_DIRS} ${Boost_LIBRARY_DIRS})

add_custom_target(all_examples)

macro(add name folder)
  add_executable(${name} ${folder}/${name}.cpp ${ARGN})
  target_link_libraries(${name} ${LD_FLAGS} ${CMAKE_DL_LIBS} ${CPPA_LIBRARY} ${PTHREAD_LIBRARIES})
  add_dependencies(${name} all_examples)
endmacro()

add(announce_1 type_system)
add(announce_2 type_system)
add(announce_3 type_system)
add(announce_4 type_system)
add(announce_5 type_system)
add(dancing_kirby message_passing)
add(dining_philosophers message_passing)
add(hello_world .)
add(aout .)
add(calculator message_passing)
add(typed_calculator message_passing)
add(distributed_calculator remote_actors)
add(group_server remote_actors)
add(group_chat remote_actors)

if (NOT "${CPPA_NO_PROTOBUF_EXAMPLES}" STREQUAL "yes")
  find_package(Protobuf)
  if (PROTOBUF_FOUND)
    PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders "${CMAKE_CURRENT_SOURCE_DIR}/remote_actors/pingpong.proto")
    include_directories(${PROTOBUF_INCLUDE_DIR})
    # add binary dir as include path as generated headers will be located there
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    add_executable(protobuf_broker remote_actors/protobuf_broker.cpp ${ProtoSources})
    target_link_libraries(protobuf_broker ${CMAKE_DL_LIBS} ${LD_DIRS} ${CPPA_LIBRARY} ${PTHREAD_LIBRARIES} ${PROTOBUF_LIBRARIES})
    add_dependencies(protobuf_broker all_examples)
  endif (PROTOBUF_FOUND)
endif ()
