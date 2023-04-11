
# 一 启用FlexFEC：
在main.cc中添加
```
constexpr char kVideoFlexfecFieldTrial[] = "WebRTC-FlexFEC-03-Advertised/Enabled";
  webrtc::field_trial::InitFieldTrialsFromString(kVideoFlexfecFieldTrial); 
```
# 二 SDP
```
a=rtpmap:96 H264/90000
a=rtpmap:35 flexfec-03/90000 //新版本是35
a=rtcp-fb:35 goog-remb
a=rtcp-fb:35 transport-cc
a=fmtp:35 repair-window=10000000
a=ssrc-group:FID 3793049148 2533540925
a=ssrc-group:FEC-FR 3793049148 3529981269  //后面是flexfec的ssrc
```
没有Red头。


 抓包可以看到：H264 media包，Flexfec包，不同ssrc发送的。

# 三 实现
码率探测后，得到protection_factor值。一帧视频可以分成几个media包，
(num_media_packets * protection_factor + (1 << 7)) >> 8，算出fec包个数。
根据随机、还是突发，查相应的mask表。默认是kFecMaskRandom。实际中还是突发丢包多。

算出那个fec保护那几个media包。
```
#define kMaskBursty7_4 \
  0x38, 0x00, \
  0x8a, 0x00, \
  0xc4, 0x00, \
  0x62, 0x00
```
7个media包，4个fec包，是kMaskBursty7_4。0x38二进制是0x0011 1000，从左边数，从第一个包开始，第一个fec包是第3、4、5包异或。
```
if (packet_masks_[pkt_mask_idx] & (1 << (7 - media_pkt_idx))) {
```
media包个数<=16，mask 2字节，查表获得。media包<=48，mask 6字节，代码生成mask表。
1字节能表示8个。最大能保护48个media包，超过了，就不参与异或了。


目前仅支持行异或--适用随机情况，仅支持R、F=0的情况。R=0不支持重发、F=0表示灵活的mask表定义形式。

 3.1 发送+编码
 ```c++
RtpSenderEgress::SendPacket()
--| FlexfecSender::AddPacketAndGenerateFec(
   --| UlpfecGenerator::AddPacketAndGenerateFec(
       mark为true，ForwardErrorCorrection::EncodeFec(
       {
          1 计算fec包个数   
          int num_fec_packets = NumFecPackets(num_media_packets, protection_factor);
          
          2 计算mask表
 
          3 // Write FEC packets to |generated_fec_packets_|.           
            GenerateFecPayloads(media_packets, num_fec_packets);
            //根据mask表，确定异或那几个包。
            第一个保护的包拷贝，后面保护的包，跟前面的异或头、包体
 
          4 写flexfec头，主要是seq_num_base、mask表
            FinalizeFecHeaders(num_fec_packets, media_ssrc, seq_num_base);
       }
 ```
 3.2 接收+解码
 ```c++
RtpVideoStreamReceiver2::OnRtpPacket(
1 RtpVideoStreamReceiver2::ReceivePacket(
  --| RtpVideoStreamReceiver2::OnInsertedPacket(--组帧
2 config_.rtp.packet_sink_->OnRtpPacket(packet);
  即FlexfecReceiveStreamImpl::OnRtpPacket(
  --| FlexfecReceiver::OnRtpPacket(
      --| 1 AddReceivedPacket：判断ssrc是否相等，相等is_fec = true。
          2 ProcessReceivedPackets()
            {
             2.1 ForwardErrorCorrection::DecodeFec(...)
             {
                //把这个FEC包保护的media包push进protected_packets队列。
                //给protected_packet->seq_num等赋值，pkt = nullptr;
                2.1.1 ForwardErrorCorrection::InsertPacket()
                
                2.1.2  ForwardErrorCorrection::AttemptRecovery()
                //根据pkt==nullptr判断丢包个数
				if (protected_packet->pkt == nullptr) 
                packets_missing>1恢复不了。=0不用恢复。
                if (packets_missing == 1) {
                ForwardErrorCorrection::RecoverPacket()：
				StartPacketRecovery(fec_packet, recovered_packet)：把fec_packet的头部分拷贝给recovered_packet
                for(fec_packet.protected_packets){ //protected_packets的包都参与异或
				  XorHeaders、XorPayloads
				}
                FinishPacketRecovery(fec_packet, recovered_packet)：recovered_packet是恢复的包、改ssrc等
                
				2.1.3 UpdateCoveringFecPackets(*recovered_packet_ptr);//根据seq给pkt赋值
                
             }
			 
			 2.2 RtpVideoStreamReceiver2::OnRecoveredPacket()//参数就是恢复的数据
			     --| RtpVideoStreamReceiver2::OnReceivedPayloadData(//继续后面接收的处理
				   //恢复出来的包，多长时间是有效的？

```                   
写的最大支持丢包50%，Restrict packet loss range to 50

在ForwardErrorCorrection::AttemptRecovery中，We can only recover one packet with an FEC packet。1个fec包恢复1个丢包。

# 四 自问自答

1 异或那些数据？

Flexfec header的前8字节：由media包的前8字节异或得到。

视频数据--data[20]。

代码见ForwardErrorCorrection::XorHeaders。

RTP头+RTP扩展头+FlexFEC header+video data。