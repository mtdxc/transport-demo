/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/1 3:45 下午
 * @mail        : qw225967@github.com
 * @project     : nack_test
 * @file        : transport-fd.h
 * @description : TODO
 *******************************************************/
#pragma once

#ifndef NACK_TEST_TRANSPORT_FD_H
#define NACK_TEST_TRANSPORT_FD_H

#include <iostream>


//#include "../third_party/linux/boost/include/boost/asio.hpp"
#include "asio.hpp"

namespace transportdemo {
  namespace PosixTime = boost::posix_time;
  typedef asio::ip::udp::endpoint  UDPEndpoint;
  typedef asio::ip::udp::endpoint  UDPEndpoint;
  typedef asio::ip::address_v4     Address;
  typedef asio::io_service         IOService;
  typedef asio::steady_timer     DeadlineTimer;
  typedef asio::error_code       ErrorCode;
  typedef asio::ip::udp::socket    UDPSocket;
  typedef std::shared_ptr<UDPSocket>      UDPSocketPrt;
} // transport-demo

#endif //NACK_TEST_TRANSPORT_FD_H
