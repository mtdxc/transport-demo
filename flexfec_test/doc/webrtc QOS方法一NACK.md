# NACK实现
[https://tools.ietf.org/html/rfc4585](https://tools.ietf.org/html/rfc4585)


## 1.1 概念

与NACK对应的是ACK，ACK是到达通知技术。以TCP为例，他可靠因为接收方在收到数据后会给发送方返回一个“已收到数据”的消息（ACK），告诉发送方“我已经收到了”，确保消息的可靠。

NACK也是一种通知技术，只是触发通知的条件刚好的ACK相反，在未收到消息时，通知发送方“我未收到消息”，即通知未达。

在rfc4585协议中定义可重传未到达数据的类型有二种：

1. RTPFB：rtp报文丢失重传；
2. PSFB：指定净荷重传，指定净荷重传里面又分如下三种：  
    - PLI    (Picture Loss Indication) 视频帧丢失重传。  
    - SLI    (Slice Loss Indication)    slice丢失重转。  
    - RPSI (Reference Picture Selection Indication)参考帧丢失重传。

在创建视频连接的SDP协议里面，会协商以上述哪种类型进行NACK重转。以webrtc为例，会协商两种NACK，一个rtp报文丢包重传的nack（nack后面不带参数，默认RTPFB）、PLI 视频帧丢失重传的nack。

![](https://img-blog.csdn.net/20180731102210958?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

## 1.2 定义

```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |V=2|P|   FMT   |       PT      |          length               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                  SSRC of packet sender                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                  SSRC of media source                         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   :            Feedback Control Information (FCI)                 :
   :                                                               :

           Figure 3: Common Packet Format for Feedback Messages
```

- V：2bit 目前固定为2
- P：1bit padding  
- PT：8bit Payload type。
  - 205 RTPFB trasnport layer FB message
  - 206 PSFB Payload-specific FB messsage

- FMT：5bit Feedback message type。

  RTP FP模式下定义值为：
  - 0: unassigned
  - 1: Generic NACK
  - 2-30: unassigned
  - 31: reserved for future expansion of the identifier number space

  PS FP模式下定义值为：
  - 0:     unassigned  
  - 1:     Picture Loss Indication (PLI)  
  - 2:     Slice Loss Indication (SLI)  
  - 3:     Reference Picture Selection Indication (RPSI)  
  - 4-14:  unassigned  
  - 15:    Application layer FB (AFB) message  
  - 16-30: unassigned  
  - 31:    reserved for future expansion of the sequence number space

- FCI：变长 Feedback Control Information。

### 1、RTPFB

```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |            PID                |             BLP               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

               Figure 4: Syntax for the Generic NACK message
```
Packet identifier(PID)即为丢失RTP数据包的序列号，Bitmap of Lost Packets(BLP)指示从PID开始接下来16个RTP数据包的丢失情况。一个NACK报文可以携带多个RTP序列号，NACK接收端对这些序列号逐个处理。如下示例：

![](https://img-blog.csdn.net/20180726143955415?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

- Packet identifier(PID)为176。
- Bitmap of Lost Packets(BLP)：0x6ae1。解析的时候需要按照小模式解析，
          0x6ae1对应二进制：110101011100001倒过来看1000 0111 0101 0110。
          按照1bit是丢包，0bit是没有丢包解析，丢失报文序列号分别是：
          177 182 183 184 186 188 190 191与wireshark解析一致。

### 2、PSFB

1）PLI FB  PT=PSFB FMT=1。

![](https://img-blog.csdn.net/20180806105718779?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

2）SLI FB的PT=PSFB、FMT=2。

```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |            First        |        Number           | PictureID |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            Figure 6: Syntax of the Slice Loss Indication (SLI)
```
- First: 13 bits The macroblock (MB) address of the first lost macroblock.
- Number: 13 bits The number of lost macroblocks, in scan order as discussed above。
- PictureID: 6 bits The six least significant bits of the codec-specific identifier that is used to reference the picture in which the loss of the macroblock(s) has occurred. For many video codecs, the PictureID is identical to the Temporal Reference.

3）RPSI FB的PT=PSFB、FMT=3。

```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      PB       |0| Payload Type|    Native RPSI bit string     |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   defined per codec          ...                | Padding (0) |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   Figure 7: Syntax of the Reference Picture Selection Indication (RPSI)
```

## 1.3 实现

webrtc支持RTPFB和PLI FB两种重传方式。

AssignPayloadTypesAndAddAssociatedRtxCodecs->AddDefaultFeedbackParams里面将两种方式都填写到SDP命令行里面。

![](https://img-blog.csdn.net/20180731151458610?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

### 1.3.1 RTPFB实现

RTPFB在JB里面实现。通过RTP报文的序列号和时间戳，判断是否出现丢包异常。参考NackTracker类实现。

发送端调用栈参考如下：  
PlatformThread::StartThread  
\->PlatformThread::Run  
\->ProcessThreadImpl::Run  
\->ProcessThreadImpl::Process  
\->PacedSender::Process  
\->PacedSender::SendPacket  
\->PacketRouter::TimeToSendPacket  
\->ModuleRtpRtcpImpl::TimeToSendPacket  
\->RTPSender::TimeToSendPacket  
\->RtpPacketHistory::GetPacketAndSetSendTime  
\->RtpPacketHistory::GetPacket

  
接收端有两种方式驱动方式NACK

1、收包驱动  
DeliverPacket  
\->DeliverRtp  
\->RtpStreamReceiverController::OnRtpPacket  
\->RtpDemuxer::OnRtpPacket  
\->RtpVideoStreamReceiver::OnRtpPacket  
\->RtpVideoStreamReceiver::ReceivePacket  
\->RtpReceiverImpl::IncomingRtpPacket  
\->RTPReceiverVideo::ParseRtpPacket  
\->RtpVideoStreamReceiver::OnReceivedPayloadData  
\->NackModule::OnReceivedPacket  
\->VideoReceiveStream::SendNack  
\->RtpVideoStreamReceiver::RequestPacketRetransmit  
\->ModuleRtpRtcpImpl::SendNack

2、定时驱动

NackModule::Process

### 1.3.2 PLI FB实现

PLI FB在webrtc里面实现的是请求关键帧。当连续出现解码失败，或者长期没有解码输入，就通过RTCP报文发送请求IDR帧命令。参考VideoReceiveStream::Decode、RequestKeyFrame这两个函数实现。


# 2. 接收端NACK流程实现

webrtc接收端触发发送NACK报文有两处：

1. 接收RTP报文，对序列号进行检测，发现有丢包，立即触发发送NACK报文。
2. 定时检查nack_list 队列，发现丢包满足申请重传条件，立即触发发送NACK报文。

## 2.1 函数实现

### 2.1.1 接收丢包触发函数实现

```cpp
NackModule::OnReceivedPacket
->NackModule::GetNackBatch
```

函数里实现，该函数在整个调用栈的位置如下：

![](https://img-blog.csdnimg.cn/20200630100041985.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

RtpVideoStreamReceiver::OnReceivedPayloadData 

![](https://img-blog.csdnimg.cn/5316b705a6ed4cbc87e39afe4f87d398.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

### 2.1.2 定时检查触发函数实现

```cpp
PlatformThread::StartThread()
->PlatformThread::Run()
->ProcessThreadImpl::Process()
->NackModule::Process()
->NackModule::GetNackBatch
```

其中NackModule::Process是挂载在接收RTP报文线程的一个定时任务。在RtpVideoStreamReceiver::RtpVideoStreamReceiver函数实现挂载。

![](https://img-blog.csdnimg.cn/20200630170246697.png)

NackModule::Process函数的调度周期是kProcessIntervalMs（默认20ms）

### 2.2 核心函数

NackModule::AddPacketsToNack、NackModule::GetNackBatch是NACK核心函数。

#### 2.2.1 NackModule::AddPacketsToNack

决定是否将该报文放入NACK队列

```cpp
void NackModule::AddPacketsToNack(uint16_t seq_num_start,
                                  uint16_t seq_num_end) {
  // Remove old packets.
  auto it = nack_list_.lower_bound(seq_num_end - kMaxPacketAge);
  nack_list_.erase(nack_list_.begin(), it);
 
  // If the nack list is too large, remove packets from the nack list until
  // the latest first packet of a keyframe. If the list is still too large,
  // clear it and request a keyframe.
  uint16_t num_new_nacks = ForwardDiff(seq_num_start, seq_num_end);
  if (nack_list_.size() + num_new_nacks > kMaxNackPackets) {
    while (RemovePacketsUntilKeyFrame() &&
           nack_list_.size() + num_new_nacks > kMaxNackPackets) {
    }
 
    if (nack_list_.size() + num_new_nacks > kMaxNackPackets) {
      nack_list_.clear();
      keyframe_request_sender_->RequestKeyFrame();
      return;
    }
  }
 
  for (uint16_t seq_num = seq_num_start; seq_num != seq_num_end; ++seq_num) {
    // Do not send nack for packets that are already recovered by FEC or RTX
    if (recovered_list_.find(seq_num) != recovered_list_.end())
      continue;
    NackInfo nack_info(seq_num, seq_num + WaitNumberOfPackets(0.5),
                       clock_->TimeInMilliseconds());
    RTC_DCHECK(nack_list_.find(seq_num) == nack_list_.end());
    nack_list_[seq_num] = nack_info;
  }
}
```

该函数的中心思想是：

1. nack_list的最大长度为kMaxNackPackets，即本次发送的nack包至多可以对kMaxNackPackets个丢失的包进行重传请求。如果丢失的包数量超过kMaxNackPackets，会循环清空nack_list中关键帧之前的包，直到其长度小于kMaxNackPackets。也就是说，放弃对关键帧首包之前的包的重传请求，直接而快速的以关键帧首包之后的包号作为重传请求的开始。
2. nack_list中包号的距离不能超过kMaxPacketAge个包号。即nack_list中的包号始终保持 \[cur\_seq\_num - kMaxPacketAge, cur\_seq\_num\] 这样的跨度，以保证nack请求列表中不会有太老旧的包号。

#### 2.2.2 NackModule::GetNackBatch

决定是否发生NACK请求重传该报文。两种触发方式都是调用这个函数决定是否发送NACK请求重传。

```cpp
std::vector<uint16_t> NackModule::GetNackBatch(NackFilterOptions options) {
  bool consider_seq_num = options != kTimeOnly;
  bool consider_timestamp = options != kSeqNumOnly;
  Timestamp now = clock_->CurrentTime();
  std::vector<uint16_t> nack_batch;
  auto it = nack_list_.begin();
  while (it != nack_list_.end()) {
    TimeDelta resend_delay = TimeDelta::Millis(rtt_ms_);
    if (backoff_settings_) {
      resend_delay = std::max(resend_delay, backoff_settings_->min_retry_interval);
      if (it->second.retries > 1) {
        TimeDelta exponential_backoff =
            std::min(TimeDelta::Millis(rtt_ms_), backoff_settings_->max_rtt) *
            std::pow(backoff_settings_->base, it->second.retries - 1);
        resend_delay = std::max(resend_delay, exponential_backoff);
      }
    }
 
    bool delay_timed_out =
        now.ms() - it->second.created_at_time >= send_nack_delay_ms_;
    bool nack_on_rtt_passed =
        now.ms() - it->second.sent_at_time >= resend_delay.ms();
    bool nack_on_seq_num_passed =
        it->second.sent_at_time == -1 &&
        AheadOrAt(newest_seq_num_, it->second.send_at_seq_num);
    if (delay_timed_out && ((consider_seq_num && nack_on_seq_num_passed) ||
                            (consider_timestamp && nack_on_rtt_passed))) {
      nack_batch.emplace_back(it->second.seq_num);
      ++it->second.retries;
      it->second.sent_at_time = now.ms();
      if (it->second.retries >= kMaxNackRetries) {
        it = nack_list_.erase(it);
      } else {
        ++it;
      }
      continue;
    }
    ++it;
  }
  return nack_batch;
}
```

该函数的中心思想是：

1. 因为报文有可能出现乱序抖动情况，不能说检测出丢包就立即重传，需要等待send_nack_delay_ms_，当等待时间大于send_nack_delay_ms_，申请重传。
2. 由于NACK的延时主要在RTT环路延时上，因此再次重传的时间一定要大于rtt_ms_，当两次发送NACK重传请求时间大于rtt_ms_时，才会申请再次重传。
3. 视频会议场景对实时性要求很高，当报文一直处于丢包状态，不能持续申请重传，最大重传次数为kMaxNackRetries，超过最大重传次数，放弃该报文。不再重传。

# 3 发送端NACK实现

![](https://img-blog.csdnimg.cn/53c146b19358411db2053cbefc4c5f78.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

## 3.1 函数走读

发送端实现NACK的三个重点流程：

### 3.1.1 发送RTP报文，实时存储报文到packet_history 队列

ProcessThreadImpl::Process  
\->PacedSender::Process      
\->PacingController::ProcessPackets  
\->PacketRouter::SendPacket  
\->ModuleRtpRtcpImpl2::TrySendPacket  
\->RtpSenderEgress::SendPacket

每次pacer发送报文的时候，都会把媒体报文储存在packet_history_队列。

![](https://img-blog.csdnimg.cn/f9247e298ab44c93bf4d230d675ae75f.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

 RtpPacketHistory::PutRtpPacket以SequenceNumber为索引，把rtp保存在packet_history_队列

![](https://img-blog.csdnimg.cn/7078f4726912417ab3ab6c01c75ef941.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

SetStorePacketsStatus配置队列长度。

视频在CreateRtpStreamSenders->SetStorePacketsStatus配置。

![](https://img-blog.csdnimg.cn/d389e61f937d44dcae436f26bb6895c7.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

 音频在RegisterSenderCongestionControlObjects->SetStorePacketsStatus配置。

![](https://img-blog.csdnimg.cn/86c01a30fadf4b89b2ad0e97bd3ad173.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

### 3.1.2 处理接收到的RTCP NACK报文

函数调用关系如下：

![](https://img-blog.csdnimg.cn/6071c4ee46564c5989f7c42e12b69b9d.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

 RTCPReceiver::HandleNack：压栈packet\_information->nack\_sequence\_numbers丢包队列。

![](https://img-blog.csdnimg.cn/20210728114108624.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

ModuleRtpRtcpImpl::OnReceivedNack

将RTT延时时间及nack\_sequence\_numbers队列更新到RTPSender::OnReceivedNack

![](https://img-blog.csdnimg.cn/046a681e01cc41e08ef5132892111bbe.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

### 3.1.3 重发NACK反馈的RTP报文

RTPSender::OnReceivedNack

![](https://img-blog.csdnimg.cn/20210728114326646.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

重发报文这里有三点需要注意：

1）GetPacketAndMarkAsPending会判断上次重传报文时间和当前时间差是否大于RTT，若小于则不重传。

![](https://img-blog.csdnimg.cn/1987e1a3c12043508b61b96049804ac6.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

RtpPacketHistory::VerifyRtt 

 ![](https://img-blog.csdnimg.cn/33d3aab2a8324bf186e38de418df2906.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

 2）NACK重新发送媒体数据有两种方式：单独RTX通道发送、与媒体数据混在一起发送

两种形式对单纯的NACK抗性影响不太大，但是与媒体数据混在一起发送模式，接收端无法区分是NACK重传报文，还是正常媒体数据，会导致接收端反馈的丢包率低于实际值，影响gcc探测码率，及发送端FEC冗余度配置。所以建议还是以RTX通道单独发送。 

![](https://img-blog.csdnimg.cn/ea75fa5c1c2044b9b4531c19ee3bff21.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

RTX通道单独发送重传报文，需要配置参数有如下三个：

![](https://img-blog.csdnimg.cn/c95d01aaa5c24f548c1faa9e8decfbe8.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

3）RTPSender::ReSendPacket在将重传数据加入pacer队列，会设置报文[优先级](https://so.csdn.net/so/search?q=%E4%BC%98%E5%85%88%E7%BA%A7&spm=1001.2101.3001.7020)，为了保证实时性，NACK重传报文需要按照高优先级重传。

优先级配置在set\_packet\_type，发送报文时，会根据kRetransmission获取发送优先级。

![](https://img-blog.csdnimg.cn/6a62d2ca57e844968ffbb9a21ae3abd1.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

 PacingController::EnqueuePacket

![](https://img-blog.csdnimg.cn/0f6d63d4c35c415688411b66bc6f5c37.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)

GetPriorityForType

![](https://img-blog.csdnimg.cn/2347b1c312b5476dae2392ee52aa0413.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAQ3J5c3RhbFNoYXc=,size_20,color_FFFFFF,t_70,g_se,x_16)