cmake_minimum_required(VERSION 3.20)
project(nack_test)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BOOST_DIR /opt/homebrew/Cellar/boost/1.76.0)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
message(STATUS ${CMAKE_BUILD_TYPE})
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")


add_definitions(-DASIO_STANDALONE)
include_directories(
        ${CMAKE_SOURCE_DIR}/../../asio/asio/include
        ${CMAKE_SOURCE_DIR}/Transport
        ${CMAKE_SOURCE_DIR}/Transport/boost-asio
)

link_directories(
        ${CMAKE_BOOST_DIR}/lib
)

add_executable(
        nack_test
        main.cpp
        Transport/pack.cpp
        Transport/boost-asio/udp_sender.cpp
)
