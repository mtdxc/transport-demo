/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_RTP_RTCP_SOURCE_ULPFEC_RECEIVER_IMPL_85_H_
#define MODULES_RTP_RTCP_SOURCE_ULPFEC_RECEIVER_IMPL_85_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <vector>

#include "modules/rtp_rtcp/include/rtp_header_extension_map.h"
#include "modules/rtp_rtcp/include/rtp_rtcp_defines.h"
#include "modules/rtp_rtcp/include/ulpfec_receiver_85.h"
#include "modules/rtp_rtcp/source/forward_error_correction_85.h"
#include "modules/rtp_rtcp/source/rtp_packet_received_85.h"
#include "rtc_base/critical_section.h"

namespace webrtc85 {

class UlpfecReceiverImpl : public UlpfecReceiver {
 public:
  explicit UlpfecReceiverImpl(uint32_t ssrc,
                              webrtc::RecoveredPacketReceiver* callback,
                              rtc::ArrayView<const webrtc::RtpExtension> extensions);
  ~UlpfecReceiverImpl() override;

  bool AddReceivedRedPacket(const RtpPacketReceived& rtp_packet,
                            uint8_t ulpfec_payload_type) override;

  int32_t ProcessReceivedFec() override;

  FecPacketCounter GetPacketCounter() const override;

 private:
  const uint32_t ssrc_;
  const webrtc::RtpHeaderExtensionMap extensions_;

  rtc::CriticalSection crit_sect_;
  webrtc::RecoveredPacketReceiver* recovered_packet_callback_;
  std::unique_ptr<webrtc85::ForwardErrorCorrection> fec_;
  // TODO(nisse): The AddReceivedRedPacket method adds one or two packets to
  // this list at a time, after which it is emptied by ProcessReceivedFec. It
  // will make things simpler to merge AddReceivedRedPacket and
  // ProcessReceivedFec into a single method, and we can then delete this list.
  std::vector<std::unique_ptr<webrtc85::ForwardErrorCorrection::ReceivedPacket>>
      received_packets_;
  webrtc85::ForwardErrorCorrection::RecoveredPacketList recovered_packets_;
  FecPacketCounter packet_counter_;
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_ULPFEC_RECEIVER_IMPL_85_H_
