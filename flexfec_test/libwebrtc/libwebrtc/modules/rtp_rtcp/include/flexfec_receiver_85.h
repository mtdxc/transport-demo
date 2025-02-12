/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_RTP_RTCP_INCLUDE_FLEXFEC_RECEIVER_85_H_
#define MODULES_RTP_RTCP_INCLUDE_FLEXFEC_RECEIVER_85_H_

#include <stdint.h>

#include <memory>

#include "modules/rtp_rtcp/include/rtp_rtcp_defines.h"
#include "modules/rtp_rtcp/include/ulpfec_receiver_85.h"
#include "modules/rtp_rtcp/source/forward_error_correction_85.h"
#include "modules/rtp_rtcp/source/rtp_packet_received_85.h"
#include "rtc_base/synchronization/sequence_checker.h"
#include "rtc_base/thread_annotations.h"

namespace webrtc85 {

class Clock;

class FlexfecReceiver {
 public:
  FlexfecReceiver(uint32_t ssrc,
                  uint32_t protected_media_ssrc,
                  webrtc::RecoveredPacketReceiver* recovered_packet_receiver);
  FlexfecReceiver(webrtc::Clock* clock,
                  uint32_t ssrc,
                  uint32_t protected_media_ssrc,
                  webrtc::RecoveredPacketReceiver* recovered_packet_receiver);
  ~FlexfecReceiver();

  // Inserts a received packet (can be either media or FlexFEC) into the
  // internal buffer, and sends the received packets to the erasure code.
  // All newly recovered packets are sent back through the callback.
  void OnRtpPacket(const RtpPacketReceived& packet);

  // Returns a counter describing the added and recovered packets.
  FecPacketCounter GetPacketCounter() const;

  // Protected to aid testing.
 protected:
  std::unique_ptr<webrtc85::ForwardErrorCorrection::ReceivedPacket> AddReceivedPacket(
      const RtpPacketReceived& packet);
  void ProcessReceivedPacket(
      const webrtc85::ForwardErrorCorrection::ReceivedPacket& received_packet);

 private:
  // Config.
  const uint32_t ssrc_;
  const uint32_t protected_media_ssrc_;

  // Erasure code interfacing and callback.
  std::unique_ptr<webrtc85::ForwardErrorCorrection> erasure_code_
      RTC_GUARDED_BY(sequence_checker_);
  webrtc85::ForwardErrorCorrection::RecoveredPacketList recovered_packets_
      RTC_GUARDED_BY(sequence_checker_);
  webrtc::RecoveredPacketReceiver* const recovered_packet_receiver_;

  // Logging and stats.
  webrtc::Clock* const clock_;
  int64_t last_recovered_packet_ms_ RTC_GUARDED_BY(sequence_checker_);
  FecPacketCounter packet_counter_ RTC_GUARDED_BY(sequence_checker_);

  webrtc::SequenceChecker sequence_checker_;
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_INCLUDE_FLEXFEC_RECEIVER_85_H_
