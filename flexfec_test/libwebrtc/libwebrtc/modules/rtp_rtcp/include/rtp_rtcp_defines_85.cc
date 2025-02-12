///*
// *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
// *
// *  Use of this source code is governed by a BSD-style license
// *  that can be found in the LICENSE file in the root of the source
// *  tree. An additional intellectual property rights grant can be found
// *  in the file PATENTS.  All contributing project authors may
// *  be found in the AUTHORS file in the root of the source tree.
// */
//
//#include "modules/rtp_rtcp/include/rtp_rtcp_defines_85.h"
//
//#include <ctype.h>
//#include <string.h>
//
//#include <type_traits>
//
//#include "absl/algorithm/container.h"
//#include "api/array_view.h"
//#include "modules/rtp_rtcp/source/rtp_packet_85.h"
//
//namespace webrtc85 {
//
//namespace {
//constexpr size_t kMidRsidMaxSize = 16;
//
//// Check if passed character is a "token-char" from RFC 4566.
//bool IsTokenChar(char ch) {
//  return ch == 0x21 || (ch >= 0x23 && ch <= 0x27) || ch == 0x2a || ch == 0x2b ||
//         ch == 0x2d || ch == 0x2e || (ch >= 0x30 && ch <= 0x39) ||
//         (ch >= 0x41 && ch <= 0x5a) || (ch >= 0x5e && ch <= 0x7e);
//}
//}  // namespace
//
//bool IsLegalMidName(absl::string_view name) {
//  return (name.size() <= kMidRsidMaxSize && !name.empty() &&
//          absl::c_all_of(name, IsTokenChar));
//}
//
//bool IsLegalRsidName(absl::string_view name) {
//  return (name.size() <= kMidRsidMaxSize && !name.empty() &&
//          absl::c_all_of(name, isalnum));
//}
//
//StreamDataCounters::StreamDataCounters() : first_packet_time_ms(-1) {}
//
//void RtpPacketCounter::AddPacket(const RtpPacket& packet) {
//  ++packets;
//  header_bytes += packet.headers_size();
//  padding_bytes += packet.padding_size();
//  payload_bytes += packet.payload_size();
//}
//
//}  // namespace webrtc
