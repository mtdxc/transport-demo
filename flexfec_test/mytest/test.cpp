/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/6/25 11:56 上午
 * @mail        : qw225967@github.com
 * @project     : flexfec
 * @file        : mytest.cpp
 * @description : TODO
 *******************************************************/

#include "test.h"
#include "modules/rtp_rtcp/source/rtp_packet.h"

#include <iostream>
#include <sstream>
#include <iomanip>

std::string bytes_to_hex(const uint8_t *buf, std::size_t len, std::size_t num_per_line = 8) {
  if (buf == NULL || len == 0 || num_per_line == 0) {
    return std::string();
  }
  std::ostringstream oss;
  for (std::size_t i = 0; i < len; i++) {
    oss << std::right << std::setw(3) << std::hex << static_cast<int>(buf[i]);
    if ((i+1) % num_per_line == 0) {
      oss << '\n';
    }
  }
  if (len % num_per_line != 0) {
    oss << '\n';
  }
  return oss.str();
}

namespace webrtc {
  test::test() {
    outfile.open("./file.txt", std::ios::out | std::ios::trunc);
    std::vector<RtpExtension> vec;
    rtc::ArrayView<const RtpExtensionSize> size;
    std::string mid("mytest");

    Clock *clock = Clock::GetRealTimeClock();
    sender_ = new FlexfecSender(113, 123123, 123123, mid, vec, size, nullptr, clock);

    FecProtectionParams delta_params;
    delta_params.fec_rate = 255;
    delta_params.max_fec_frames = 10;
    delta_params.fec_mask_type = kFecMaskRandom;

    FecProtectionParams key_params;
    key_params.fec_rate = 255;
    key_params.max_fec_frames = 10;
    key_params.fec_mask_type = kFecMaskRandom;
#if USE_RTC85
    sender_->SetProtectionParameters(delta_params, key_params);
#else
    sender_->SetFecParameters(key_params);
#endif
    receiver_ = new FlexfecReceiver(123123, 123123, this);
  }

  test::~test() {
    if (sender_)
      delete sender_;

    if (receiver_)
      delete receiver_;
    // 打印和判断seq是否连续
    uint16_t pre_seq = 0;
    for(auto iter = record_map.begin(); iter != record_map.end(); iter++) {
      if (!pre_seq) {
        pre_seq = iter->second.SequenceNumber();
      } 
      else if (iter->second.SequenceNumber() - pre_seq >= 2) {
        std::cout << "wrong seq pre_seq:" << pre_seq << ", seq:" << iter->second.SequenceNumber() << std::endl;
      }
      outfile << "rtp recover seq:" << iter->second.SequenceNumber()
      << ", payloadType:" << uint8_t(iter->second.PayloadType())
      << ", ssrc:" << iter->second.Ssrc() << std::endl;
      pre_seq = iter->second.SequenceNumber();
    }

    outfile.close();
  }


  void test::OnRecoveredPacket(const uint8_t *packet, size_t length) {
    RtpPacketReceived parsed_packet(nullptr);
    if (!parsed_packet.Parse(packet, length)) {
      std::cout << "parse err" << std::endl;
      return;
    }

//    outfile << "rtp recover seq:" << parsed_packet.SequenceNumber()
//              << ", payloadType:" << uint8_t(parsed_packet.PayloadType())
//              << ", ssrc:" << parsed_packet.Ssrc() << std::endl;

    record_map[parsed_packet.SequenceNumber()] = parsed_packet;
  }

  void test::WorkTest() {
    FILE *origin = fopen("test.264", "r");
    if (!origin) {
      return;
    }

    for (uint16_t i = 0; i < 65535; i++) {
      char s[1000];
      fgets(s, 1000, origin);

      RtpPacketToSend pkt(nullptr, 1000);
      // pkt.Parse(kPacketWithH264, sizeof(kPacketWithH264));
      pkt.SetSequenceNumber(i);
      pkt.SetPayloadType(101);
      pkt.SetSsrc(123123);
      memcpy(pkt.Buffer().data(),s,1000);
      pkt.SetPayloadSize(1000);
      if (i%48 == 0)
        pkt.SetMarker(true);
#if USE_RTC85
      sender_->AddPacketAndGenerateFec(pkt);
#else
      sender_->AddRtpPacketAndGenerateFec(pkt);
#endif
      auto vec_s = sender_->GetFecPackets();
      if (vec_s.size()) {
        for (auto iter = vec_s.begin(); iter != vec_s.end(); iter++) {
          RtpPacketReceived parsed_packet(nullptr);
          parsed_packet.Parse((*iter)->data(), (*iter)->size());
          parsed_packet.SetPayloadType(113);
          receiver_->OnRtpPacket(parsed_packet);
        }
      }

      {
        if (i%2 == 0) {
          continue;
        }
        RtpPacketReceived parsed_packet(nullptr);
        parsed_packet.Parse(pkt.data(), pkt.size());
        receiver_->OnRtpPacket(parsed_packet);
        record_map[parsed_packet.SequenceNumber()] = parsed_packet;
      }
    }

    fclose(origin);
  }
}// namespace webrtc