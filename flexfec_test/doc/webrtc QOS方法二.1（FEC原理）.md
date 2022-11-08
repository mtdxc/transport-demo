# 一、概述

[webrtc](https://so.csdn.net/so/search?q=webrtc&spm=1001.2101.3001.7020)冗余打包方式有三种：Red（rfc2198）、Ulpfec（rfc5109）、Flexfec（草案）。其中Red和Ulpfec要成对使用。

# 二、RedFEC

简单将老报文打包到新包上。如下图所示，[冗余](https://so.csdn.net/so/search?q=%E5%86%97%E4%BD%99&spm=1001.2101.3001.7020)度为1时，RFC2198打包情况：

![](https://img-blog.csdnimg.cn/20190808091945287.png)

这种方法在音视频领域几乎不使用，因为冗余包只能保护特定一个报文，这种方法带宽占用量很大，恢复能力有限，性价比很低。只是早期的T38传真、RFC2833收号会使用该协议，因为传真和收号的数据量比较小。

webrtc里面说使用了RFC2198冗余，实际上仅仅是借用该协议的封装格式，封装FEC冗余报文。

# 三、UlpFEC

详细介绍可参考：[webrtc QOS方法二.2（ulpfec rfc5109简介）\_CrystalShaw的博客-CSDN博客\_ulpfec](https://blog.csdn.net/CrystalShaw/article/details/102950002 "webrtc QOS方法二.2（ulpfec rfc5109简介）_CrystalShaw的博客-CSDN博客_ulpfec")

将一组M个报文进行异或，生成N（N就是FEC的冗余度）个FEC报文，打包出去。这组报文任意丢其中的N个，都可以通过这组(M-N)个报文+FEC冗余包恢复回来，比简单的RFC2198保护的范围扩大了很多。例如下面示意图：D为媒体包，R为冗余包，该图所示的冗余度为2。

### 1、发送端打包示意图

![](https://img-blog.csdnimg.cn/20190807173951909.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

### 2、网络丢包示意图

![](https://img-blog.csdnimg.cn/20190808092936504.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

### 3、丢包恢复示意图

![](https://img-blog.csdnimg.cn/20190807174110782.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

webrtc通过PacketMaskTable表格在选取异或模板。PacketMaskTable表格有连续丢包（kFecMaskBursty、kPacketMaskBurstyTbl）、随机丢包（kFecMaskRandom、kPacketMaskRandomTbl）两种模型。

理论上webrtc可以通过损失程度和乱序情况相关的反馈，自适应选择kFecMaskRandom还是kFecMaskBursty，效果比较好。但是可惜的是，webrtc这块功能缺失，默认使用随机丢包模型。

# 四、FlexFEC

同UlpFEC实现方式，ULPFEC仅在1D行数组上进行异或，FlexFec更灵活，引进了交织算法，可以在1D行、列、2D数组异或。

### 1、1D行异或

![](https://img-blog.csdnimg.cn/20190807173304237.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

### 2、1D列异或

![](https://img-blog.csdnimg.cn/20190807173358161.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

### 3、2D行列异或

![](https://img-blog.csdnimg.cn/20190807173506819.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

这块还是草案，如何选择异或模式的代码看没深入下去。后续补充。

需要注意，开启FlexFEC需要同时使能 WebRTC-FlexFEC-03/Enabled  && WebRTC-FlexFEC-03-Advertised/Enabled 否则会出现死机异常

# 五、FEC算法汇总

FEC是无线传输领域的一个前向纠错的算法。网上搜资料的时候经常把无线的算法看的云里雾里的，研究半天都不知道这个和视频传输有什么关系。

无线传输领域的FEC算法主要有TURBO、LDPC、POLAR这三种。

音视频传输领域的FEC算法有如下几种：

![](https://img-blog.csdnimg.cn/20190808100856334.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

1. webrtc的opus音频使用的是inband FEC和交织编码
2. webrtc的视频ulpfec使用的是异或XOR
3. Reed Solomon算法比较复杂，理论上数据恢复能力比较强。

# 六、webrtc代码分析

### 1）使能FEC

webrtc默认使能Red+Ulp的FEC。Flex仅在实验阶段，还不能正式使用。

![](https://img-blog.csdnimg.cn/20181026175541479.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_27,color_FFFFFF,t_70)

### 2）封装FEC

-   发送冗余报文处理

RTPSenderVideo::SendVideo。当编码器支持时间分层，可以仅冗余level 0的视频数据。否则，就要冗余所有视频数据。冗余度是根据丢包率动态调整。

![](https://img-blog.csdnimg.cn/20181026175404458.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_27,color_FFFFFF,t_70)

-   动态调整冗余参数调用栈
```c++
BitrateAllocator::OnNetworkChanged  
 ->VideoSendStreamImpl::OnBitrateUpdated  
 ->ProtectionBitrateCalculator::SetTargetRates  
 ->media_optimization::VCMLossProtectionLogic::UpdateMethod  
 ->media_optimization::VCMNackFecMethod::UpdateParameters    
```
-   最大保护帧数确定

  VCMNackFecMethod::ComputeMaxFramesFec

![](https://img-blog.csdnimg.cn/20191022095045628.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

-   冗余报文个数确定

ForwardErrorCorrection::NumFecPackets 存储媒体报文数\*保护因子。
```c++
int ForwardErrorCorrection::NumFecPackets(int num_media_packets,
                                          int protection_factor) {
  // Result in Q0 with an unsigned round.
  // 该算法基本保证最大48g
  int num_fec_packets = (num_media_packets * protection_factor + (1 << 7)) >> 8;
  // Generate at least one FEC packet if we need protection.
  if (protection_factor > 0 && num_fec_packets == 0) {
    num_fec_packets = 1;
  }
  RTC_DCHECK_LE(num_fec_packets, num_media_packets);
  return num_fec_packets;
}
```

-   根据丢包率动态调整冗余度

VCMFecMethod::ProtectionFactor

![](https://img-blog.csdnimg.cn/20190805161024546.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

-   根据丢包模型原则要冗余的报文

ForwardErrorCorrection::EncodeFec

![](https://img-blog.csdnimg.cn/20191028150040295.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

ForwardErrorCorrection::GenerateFecPayloads

![](https://img-blog.csdnimg.cn/2019102814583942.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0NyeXN0YWxTaGF3,size_16,color_FFFFFF,t_70)

备注：

webrtc在H264编码默认关闭ULPFEC功能，但是可以开启FlexFEC
```
MaybeCreateFecGenerator
 ->ShouldDisableRedAndUlpfec
 ->PayloadTypeSupportsSkippingFecPackets 
```
![](https://img-blog.csdnimg.cn/ca62a819a24e4379a9569c020d86841b.png)

# 参考

[RED (REDundant coding) - WebRTC Glossary](https://webrtcglossary.com/red/)

[ULPFEC (Uneven Level Protection Forward Error Correction) - WebRTC Glossary](https://webrtcglossary.com/ulpfec/)

[webrtc fec - 明明是悟空 - 博客园](https://www.cnblogs.com/x_wukong/p/8193290.html "webrtc fec - 明明是悟空 - 博客园")

[FEC算法_cloudfly_cn的博客-CSDN博客_fec算法](https://blog.csdn.net/u010178611/article/details/82656838)

[ulp-fec,flex-fec mask表解读\_zhenfei2017的博客-CSDN博客](https://blog.csdn.net/qq_16135205/article/details/89843062)（介绍冗余度和冗余Mask参数）