/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/6/25 11:56 上午
 * @mail        : qw225967@github.com
 * @project     : flexfec
 * @file        : mytest.h
 * @description : TODO
 *******************************************************/
#include "modules/rtp_rtcp/include/rtp_rtcp_defines.h"
#include "modules/rtp_rtcp/include/flexfec_receiver.h"
#include "modules/rtp_rtcp/include/flexfec_sender.h"
#include "modules/rtp_rtcp/source/rtp_packet_received.h"
#include "modules/rtp_rtcp/source/rtp_packet_to_send.h"
#include "rtp_packet_test.h"

#include <fstream>
#include <map>

#ifndef FLEXFEC_TEST_H
#define FLEXFEC_TEST_H

namespace webrtc {
  class test : webrtc::RecoveredPacketReceiver {
  public:
    test();
    ~test();

    void OnRecoveredPacket(const uint8_t *packet, size_t length) override;

    void WorkTest();

  private:
    FlexfecReceiver * receiver_;
    FlexfecSender * sender_;
    std::ofstream outfile;

    std::map<uint16_t, RtpPacketReceived> record_map;
  };
}

#endif//FLEXFEC_TEST_H
