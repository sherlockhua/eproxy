cmake_minimum_required (VERSION 2.6)
project (EPROXY)
set (EPROXY_VERSION_MAJOR 1)
set (EPROXY_VERSION_MINOR 0)

set (EPROXY_ROOT_DIR "./")
set (EPROXY "eproxy")

set(CMAKE_BUILD_TYPE "Debug")
if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-unused-variable")
else( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif( CMAKE_BUILD_TYPE STREQUAL "Debug" )

message("Build Type:"${CMAKE_BUILD_TYPE} ${CMAKE_CXX_FLAGS})

#add_definitions("-DEPROXY_DEBUG") 

#set (EPROXY_BIN "${PROJECT_ROOT_DIR}/bin/xm_rpc_test")
SET(LIBRARY_OUTPUT_PATH "./build/")

add_custom_target(libev_target ALL)

add_custom_command(TARGET libev_target 
    PRE_BUILD 
    COMMAND cd deps/libev-4.19/ && ./configure --enable-shared=no CFLAGS=-fPIC &&make clean&&make
    COMMENT "build libev"
)

include_directories("${EPROXY_ROOT_DIR}include/")
include_directories("${EPROXY_ROOT_DIR}/deps/libev-4.19/")

link_directories("${EPROXY_ROOT_DIR}deps/libev-4.19/.libs/")

SET(EXECUTABLE_OUTPUT_PATH "./bin/")
#set(GCC_EXTRA_OPTIONS "-fPIC")
set(CMAKE_CXX_FLAGS "-Wall ${GCC_EXTRA_OPTIONS} -pedantic -Wno-long-long")

file(GLOB CXX_SOURCES "./src/*.cpp")
file(GLOB CXX_INCLUDES "./include/*.h")

ADD_EXECUTABLE(${EPROXY} ${CXX_SOURCES})
target_link_libraries(${EPROXY} -lpthread ev)
