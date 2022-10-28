 前面的内容解析了拥塞控制模块，它的作用只是将网络的状况进行及时地反馈以及预测，但是如何在弱网环境下进行可靠传输并没有详细的介绍。当网络传输出现问题（例如：带宽下降、丢包上涨、延迟增加等），那么如何让用户在弱网条件下拥有更好的体验呢？答案是我们降低发送的码率，牺牲观看的清晰度并维持流畅的播放是可行的。下面我们将介绍mediasoup中两种从客户端入手进行码率调整的方案：simulcast 和 SVC（Scalable Video Coding），相关的文档可以浏览mediasoup的官方介绍文档。

# 一、Simulcast
## 1.1 simulcast简介
  simulcast直译过来是多播的意思，其实就是一个客户端发送多条不同码率大小的流，为了达到这样的效果，客户端会对同一帧画面进行不同分辨率的编码来达到降低码率的目的。

 例如上图，推流客户端同时推两路大小不一的流随后在服务器对下行网络进行估计，网络正常时下发720P分辨率的流，网络发生拥塞时下发360P分辨率的流，就能实现码率下降来抵抗弱网。

## 1.2 mediasoup中的Simulcast
  simulcast模块集成在 SimulcastConsumer 类中——SimulcastConsumer.hpp 和 SimulcastConsumer.cpp。下面给出该类的头文件：
```c++
class SimulcastConsumer : public RTC::Consumer, public RTC::RtpStreamSend::Listener
{
public:
	SimulcastConsumer(
	  const std::string& id,
	  const std::string& producerId,
	  RTC::Consumer::Listener* listener,
	  json& data);
	~SimulcastConsumer() override;

public:
	void FillJson(json& jsonObject) const override;
	void FillJsonStats(json& jsonArray) const override;
	void FillJsonScore(json& jsonObject) const override;
	void HandleRequest(Channel::Request* request) override;
	RTC::Consumer::Layers GetPreferredLayers() const override
	{
		RTC::Consumer::Layers layers;

		layers.spatial  = this->preferredSpatialLayer;
		layers.temporal = this->preferredTemporalLayer;

		return layers;
	}
	bool IsActive() const override
	{
		// clang-format off
		return (
			RTC::Consumer::IsActive() &&
			std::any_of(
				this->producerRtpStreams.begin(),
				this->producerRtpStreams.end(),
				[](const RTC::RtpStream* rtpStream)
				{
					return (rtpStream != nullptr && rtpStream->GetScore() > 0u);
				}
			)
		);
		// clang-format on
	}
	void ProducerRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc) override;
	void ProducerNewRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc) override;
	void ProducerRtpStreamScore(RTC::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
	void ProducerRtcpSenderReport(RTC::RtpStream* rtpStream, bool first) override;
	uint8_t GetBitratePriority() const override;
	uint32_t IncreaseLayer(uint32_t bitrate, bool considerLoss) override;
	void ApplyLayers() override;
	uint32_t GetDesiredBitrate() const override;
	void SendRtpPacket(RTC::RtpPacket* packet) override;
	void GetRtcp(RTC::RTCP::CompoundPacket* packet, RTC::RtpStreamSend* rtpStream, uint64_t nowMs) override;
	std::vector<RTC::RtpStreamSend*> GetRtpStreams() override
	{
		return this->rtpStreams;
	}
	void NeedWorstRemoteFractionLost(uint32_t mappedSsrc, uint8_t& worstRemoteFractionLost) override;
	void ReceiveNack(RTC::RTCP::FeedbackRtpNackPacket* nackPacket) override;
	void ReceiveKeyFrameRequest(RTC::RTCP::FeedbackPs::MessageType messageType, uint32_t ssrc) override;
	void ReceiveRtcpReceiverReport(RTC::RTCP::ReceiverReport* report) override;
	uint32_t GetTransmissionRate(uint64_t nowMs) override;
	float GetRtt() const override;

private:
	void UserOnTransportConnected() override;
	void UserOnTransportDisconnected() override;
	void UserOnPaused() override;
	void UserOnResumed() override;
	void CreateRtpStream();
	void RequestKeyFrames();
	void RequestKeyFrameForTargetSpatialLayer();
	void RequestKeyFrameForCurrentSpatialLayer();
	void MayChangeLayers(bool force = false);
	bool RecalculateTargetLayers(int16_t& newTargetSpatialLayer, int16_t& newTargetTemporalLayer) const;
	void UpdateTargetLayers(int16_t newTargetSpatialLayer, int16_t newTargetTemporalLayer);
	bool CanSwitchToSpatialLayer(int16_t spatialLayer) const;
	void EmitScore() const;
	void EmitLayersChange() const;
	RTC::RtpStream* GetProducerCurrentRtpStream() const;
	RTC::RtpStream* GetProducerTargetRtpStream() const;
	RTC::RtpStream* GetProducerTsReferenceRtpStream() const;

	/* Pure virtual methods inherited from RtpStreamSend::Listener. */
public:
	void OnRtpStreamScore(RTC::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
	void OnRtpStreamRetransmitRtpPacket(RTC::RtpStreamSend* rtpStream, RTC::RtpPacket* packet) override;

private:
	// Allocated by this.
	RTC::RtpStreamSend* rtpStream{ nullptr };
	// Others.
	std::unordered_map<uint32_t, int16_t> mapMappedSsrcSpatialLayer;
	std::vector<RTC::RtpStreamSend*> rtpStreams;
	std::vector<RTC::RtpStream*> producerRtpStreams; // Indexed by spatial layer.
	bool syncRequired{ false };
	RTC::SeqManager<uint16_t> rtpSeqManager;
	int16_t preferredSpatialLayer{ -1 };
	int16_t preferredTemporalLayer{ -1 };
	int16_t provisionalTargetSpatialLayer{ -1 };
	int16_t provisionalTargetTemporalLayer{ -1 };
	int16_t targetSpatialLayer{ -1 };
	int16_t targetTemporalLayer{ -1 };
	int16_t currentSpatialLayer{ -1 };
	int16_t tsReferenceSpatialLayer{ -1 }; // Used for RTP TS sync.
	std::unique_ptr<RTC::Codecs::EncodingContext> encodingContext;
	uint32_t tsOffset{ 0u }; // RTP Timestamp offset.
	bool keyFrameForTsOffsetRequested{ false };
	uint64_t lastBweDowngradeAtMs{ 0u }; // Last time we moved to lower spatial layer due to BWE.
};

```
这里需要提到成员命名中的空间层和时间层的概念，码率其实是一个数据大小与时间的比值，那么是两个维度的衡量值，因此我们控制发送码率可以通过两个维度去调整，一个是空间层级、一个是时间层级。与空间强相关的就是我们的分辨率——也就是每张图片的大小，而与时间强相关的则是帧率——也就是每秒多少张图，这样我们就可以控制流的大小了。
  而代码中关键成员为下列几个（我们可以直接理解空间为分辨率标志，时间为帧率标志）：
    空间层级：currentSpatialLayer（当前大小）、targetSpatialLayer（目标大小）、preferredSpatialLayer（最佳大小）、provisionalTargetSpatialLayer（临时大小）、tsReferenceSpatialLayer（参考空间）；
    时间层级：preferredTemporalLayer（最佳时间）、provisionalTargetTemporalLayer（参考时间）、targetTemporalLayer（目标时间）。

  以上的成员作为 producerRtpStreams 管理的标识来对流进行分辨，当有新的流生成时，会根据流中的参数放到producerRtpStreams中等待切换。具体代码如下：
```c++
void SimulcastConsumer::ProducerRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc)
{
	MS_TRACE();

	auto it = this->mapMappedSsrcSpatialLayer.find(mappedSsrc);
	MS_ASSERT(it != this->mapMappedSsrcSpatialLayer.end(), "unknown mappedSsrc");

	int16_t spatialLayer = it->second;
	this->producerRtpStreams[spatialLayer] = rtpStream;
}

void SimulcastConsumer::ProducerNewRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc)
{
	MS_TRACE();

	auto it = this->mapMappedSsrcSpatialLayer.find(mappedSsrc);
	MS_ASSERT(it != this->mapMappedSsrcSpatialLayer.end(), "unknown mappedSsrc");

	int16_t spatialLayer = it->second;

	this->producerRtpStreams[spatialLayer] = rtpStream;

	// Emit the score event.
	EmitScore();

	if (IsActive())
		MayChangeLayers();
}

void SimulcastConsumer::ProducerRtpStreamScore(
  RTC::RtpStream* /*rtpStream*/, uint8_t score, uint8_t previousScore)
{
	MS_TRACE();

	// Emit the score event.
	EmitScore();

	if (RTC::Consumer::IsActive())
	{
		// Just check target layers if the stream has died or reborned.
		// clang-format off
		if (
			!this->externallyManagedBitrate ||
			(score == 0u || previousScore == 0u)
		)
		// clang-format on
		{
			MayChangeLayers();
		}
	}
}

```
流存储结束后我们分析一下发送rtp数据的流程。在之前的文章中提到的拥塞控制模块中的TransportCongestionControlClient类是服务端中控制发送的部分。我们直接从feedback然后带宽估计完成后进行解析。当带宽估计完成，我们的目标码率和当前码率出现差异时将会进行下面的函数调用。
```c++
void TransportCongestionControlClient::MayEmitAvailableBitrateEvent(uint32_t previousAvailableBitrate)
{
	MS_TRACE();

	uint64_t nowMs = DepLibUV::GetTimeMsInt64();
	bool notify{ false };

	// Ignore if first event.
	// NOTE: Otherwise it will make the Transport crash since this event also happens
	// during the constructor of this class.
	if (this->lastAvailableBitrateEventAtMs == 0u)
	{
		this->lastAvailableBitrateEventAtMs = nowMs;
		return;
	}

	// Emit if this is the first valid event.
	if (!this->availableBitrateEventCalled)
	{
		this->availableBitrateEventCalled = true;
		notify = true;
	}
	// Emit event if AvailableBitrateEventInterval elapsed.
	else if (nowMs - this->lastAvailableBitrateEventAtMs >= AvailableBitrateEventInterval)
	{
		notify = true;
	}
	// Also emit the event fast if we detect a high BWE value decrease.
	else if (this->bitrates.availableBitrate < previousAvailableBitrate * 0.75)
	{
		MS_WARN_TAG(
		  bwe,
		  "high BWE value decrease detected, notifying the listener [now:%" PRIu32 ", before:%" PRIu32
		  "]",
		  this->bitrates.availableBitrate,
		  previousAvailableBitrate);

		notify = true;
	}
	// Also emit the event fast if we detect a high BWE value increase.
	else if (this->bitrates.availableBitrate > previousAvailableBitrate * 1.50)
	{
		MS_DEBUG_TAG(
		  bwe,
		  "high BWE value increase detected, notifying the listener [now:%" PRIu32 ", before:%" PRIu32 "]",
		  this->bitrates.availableBitrate,
		  previousAvailableBitrate);

		notify = true;
	}

	if (notify)
	{
		MS_DEBUG_DEV(
		  "notifying the listener with new available bitrate:%" PRIu32,
		  this->bitrates.availableBitrate);

		this->lastAvailableBitrateEventAtMs = nowMs;

		this->listener->OnTransportCongestionControlClientBitrates(this, this->bitrates);
	}
}

```

上述代码中主要干了码率增大减小时是否变更码流的阈值判断，思想就是：当码率在2s内迅速减小到75%时切换小码率、2s内码率增大到150%时切换大码率。意味着在上调码率上更为保守可以保证流畅传输，不至于因为网络传输的波动而导致来回切换。当上述的判断结束后，就会进入传输控制类的码率调整的方法中：
```c
inline void Transport::OnTransportCongestionControlClientBitrates(
  RTC::TransportCongestionControlClient* /*tccClient*/,
  RTC::TransportCongestionControlClient::Bitrates& bitrates)
{
	MS_TRACE();

	MS_DEBUG_DEV("outgoing available bitrate:%" PRIu32, bitrates.availableBitrate);

	DistributeAvailableOutgoingBitrate();
	ComputeOutgoingDesiredBitrate();

	// May emit 'trace' event.
	EmitTraceEventBweType(bitrates);
}
```

Transport类管理的所有传输控制的内容，算是一个中介者。在DistributeAvailableOutgoingBitrate函数中对切换流的具体内容进行了控制：

```c
void Transport::DistributeAvailableOutgoingBitrate()
{
	MS_TRACE();

	MS_ASSERT(this->tccClient, "no TransportCongestionClient");

	std::multimap<uint8_t, RTC::Consumer*> multimapPriorityConsumer;

	// Fill the map with Consumers and their priority (if > 0).
	for (auto& kv : this->mapConsumers)
	{
		auto* consumer = kv.second;
		auto priority  = consumer->GetBitratePriority();

		if (priority > 0u)
			multimapPriorityConsumer.emplace(priority, consumer);
	}

	// Nobody wants bitrate. Exit.
	if (multimapPriorityConsumer.empty())
		return;

	uint32_t availableBitrate = this->tccClient->GetAvailableBitrate();

	this->tccClient->RescheduleNextAvailableBitrateEvent();

	MS_DEBUG_DEV("before layer-by-layer iterations [availableBitrate:%" PRIu32 "]", availableBitrate);

	// Redistribute the available bitrate by allowing Consumers to increase
	// layer by layer. Take into account the priority of each Consumer to
	// provide it with more bitrate.
	while (availableBitrate > 0u)
	{
		auto previousAvailableBitrate = availableBitrate;

		for (auto it = multimapPriorityConsumer.rbegin(); it != multimapPriorityConsumer.rend(); ++it)
		{
			auto priority  = it->first;
			auto* consumer = it->second;
			auto bweType   = this->tccClient->GetBweType();

			// If a Consumer has priority > 1, call IncreaseLayer() more times to
			// provide it with more available bitrate to choose its preferred layers.
			for (uint8_t i{ 1u }; i <= priority; ++i)
			{
				uint32_t usedBitrate;

				switch (bweType)
				{
					case RTC::BweType::TRANSPORT_CC:
						usedBitrate = consumer->IncreaseLayer(availableBitrate, /*considerLoss*/ false);
						break;
					case RTC::BweType::REMB:
						usedBitrate = consumer->IncreaseLayer(availableBitrate, /*considerLoss*/ true);
						break;
				}

				MS_ASSERT(usedBitrate <= availableBitrate, "Consumer used more layer bitrate than given");

				availableBitrate -= usedBitrate;

				// Exit the loop fast if used bitrate is 0.
				if (usedBitrate == 0u)
					break;
			}
		}

		// If no Consumer used bitrate, exit the loop.
		if (availableBitrate == previousAvailableBitrate)
			break;
	}

	MS_DEBUG_DEV("after layer-by-layer iterations [availableBitrate:%" PRIu32 "]", availableBitrate);

	// Finally instruct Consumers to apply their computed layers.
	for (auto it = multimapPriorityConsumer.rbegin(); it != multimapPriorityConsumer.rend(); ++it)
	{
		auto* consumer = it->second;

		consumer->ApplyLayers();
	}
}

```

把待切换的码率通过关联的 consumer 类成员传入 IncreaseLayer。
```c
uint32_t SimulcastConsumer::IncreaseLayer(uint32_t bitrate, bool considerLoss)
{
	MS_TRACE();

	MS_ASSERT(this->externallyManagedBitrate, "bitrate is not externally managed");
	MS_ASSERT(IsActive(), "should be active");

	// If already in the preferred layers, do nothing.
	// clang-format off
	if (
		this->provisionalTargetSpatialLayer == this->preferredSpatialLayer &&
		this->provisionalTargetTemporalLayer == this->preferredTemporalLayer
	)
	// clang-format on
	{
		return 0u;
	}

	uint32_t virtualBitrate;

	if (considerLoss)
	{
		// Calculate virtual available bitrate based on given bitrate and our
		// packet lost.
		auto lossPercentage = this->rtpStream->GetLossPercentage();

		if (lossPercentage < 2)
			virtualBitrate = 1.08 * bitrate;
		else if (lossPercentage > 10)
			virtualBitrate = (1 - 0.5 * (lossPercentage / 100)) * bitrate;
		else
			virtualBitrate = bitrate;
	}
	else
	{
		virtualBitrate = bitrate;
	}

	uint32_t requiredBitrate{ 0u };
	int16_t spatialLayer{ 0 };
	int16_t temporalLayer{ 0 };
	auto nowMs = DepLibUV::GetTimeMs();

	for (size_t sIdx{ 0u }; sIdx < this->producerRtpStreams.size(); ++sIdx)
	{
		spatialLayer = static_cast<int16_t>(sIdx);

		// If this is higher than current spatial layer and we moved to to current spatial
		// layer due to BWE limitations, check how much it has elapsed since then.
		if (nowMs - this->lastBweDowngradeAtMs < BweDowngradeConservativeMs)
		{
			if (this->provisionalTargetSpatialLayer > -1 && spatialLayer > this->currentSpatialLayer)
			{
				MS_DEBUG_DEV(
				  "avoid upgrading to spatial layer %" PRIi16 " due to recent BWE downgrade", spatialLayer);

				goto done;
			}
		}

		// Ignore spatial layers lower than the one we already have.
		if (spatialLayer < this->provisionalTargetSpatialLayer)
			continue;

		// This can be null.
		auto* producerRtpStream = this->producerRtpStreams.at(spatialLayer);

		// Producer stream does not exist or it's not good. Ignore.
		if (!producerRtpStream || producerRtpStream->GetScore() < StreamGoodScore)
			continue;

		// If the stream has not been active time enough and we have an active one
		// already, move to the next spatial layer.
		// clang-format off
		if (
			spatialLayer != this->provisionalTargetSpatialLayer &&
			this->provisionalTargetSpatialLayer != -1 &&
			producerRtpStream->GetActiveMs() < StreamMinActiveMs
		)
		// clang-format on
		{
			continue;
		}

		// We may not yet switch to this spatial layer.
		if (!CanSwitchToSpatialLayer(spatialLayer))
			continue;

		temporalLayer = 0;

		// Check bitrate of every temporal layer.
		for (; temporalLayer < producerRtpStream->GetTemporalLayers(); ++temporalLayer)
		{
			// Ignore temporal layers lower than the one we already have (taking into account
			// the spatial layer too).
			// clang-format off
			if (
				spatialLayer == this->provisionalTargetSpatialLayer &&
				temporalLayer <= this->provisionalTargetTemporalLayer
			)
			// clang-format on
			{
				continue;
			}

			requiredBitrate = producerRtpStream->GetLayerBitrate(nowMs, 0, temporalLayer);

			// This is simulcast so we must substract the bitrate of the current temporal
			// spatial layer if this is the temporal layer 0 of a higher spatial layer.
			//
			// clang-format off
			if (
				requiredBitrate &&
				temporalLayer == 0 &&
				this->provisionalTargetSpatialLayer > -1 &&
				spatialLayer > this->provisionalTargetSpatialLayer
			)
			// clang-format on
			{
				auto* provisionalProducerRtpStream =
				  this->producerRtpStreams.at(this->provisionalTargetSpatialLayer);
				auto provisionalRequiredBitrate = provisionalProducerRtpStream->GetLayerBitrate(
				  nowMs, 0, this->provisionalTargetTemporalLayer);

				if (requiredBitrate > provisionalRequiredBitrate)
					requiredBitrate -= provisionalRequiredBitrate;
				else
					requiredBitrate = 1u; // Don't set 0 since it would be ignored.
			}

			MS_DEBUG_DEV(
			  "testing layers %" PRIi16 ":%" PRIi16 " [virtual bitrate:%" PRIu32
			  ", required bitrate:%" PRIu32 "]",
			  spatialLayer,
			  temporalLayer,
			  virtualBitrate,
			  requiredBitrate);

			// If active layer, end iterations here. Otherwise move to next spatial layer.
			if (requiredBitrate)
				goto done;
			else
				break;
		}

		// If this is the preferred or higher spatial layer, take it and exit.
		if (spatialLayer >= this->preferredSpatialLayer)
			break;
	}

done:

	// No higher active layers found.
	if (!requiredBitrate)
		return 0u;

	// No luck.
	if (requiredBitrate > virtualBitrate)
		return 0u;

	// Set provisional layers.
	this->provisionalTargetSpatialLayer  = spatialLayer;
	this->provisionalTargetTemporalLayer = temporalLayer;

	MS_DEBUG_DEV(
	  "setting provisional layers to %" PRIi16 ":%" PRIi16 " [virtual bitrate:%" PRIu32
	  ", required bitrate:%" PRIu32 "]",
	  this->provisionalTargetSpatialLayer,
	  this->provisionalTargetTemporalLayer,
	  virtualBitrate,
	  requiredBitrate);

	if (requiredBitrate <= bitrate)
		return requiredBitrate;
	else if (requiredBitrate <= virtualBitrate)
		return bitrate;
	else
		return requiredBitrate; // NOTE: This cannot happen.
}

```

而这个函数是对层级的切换控制函数，主要思想是：通过对流的评分系统来挑选出一个适合的流，然后把流切换到目标中。最后是调用SendRtpPacket函数发送完成。

## 1.3 Simulcast的缺陷
  虽然Simulcast可以解决下行带宽不足时降码率的需求，但是在应用的过程中存在比较致命的缺陷：
  1.上行带宽的增加，我们在上行时会多发送多条流，导致大量的带宽浪费。而为了应对几个下行网络较差的用户而牺牲上行用户的带宽资源，这样的做法有待商榷；
  2.下行可供的流选择性很少，及时使用3条流同时传输的方式，在移动网络这样复杂多变的网络条件下，上下调整的幅度将会是巨大的，会造成不好的用户体验，相比SVC适用性较差。

# 二、SVC
## 2.1 SVC简介
  可伸缩视频编码SVC（Scalable Video Coding）技术是H.264标准的一个扩展，最初由JVT在2004年开始制定。H.264 SVC是H.264标准的扩展部分，SVC扩展部分引入了一种传统H.264 AVC不存在的概念——编码流中的层。基本层编码最低层的时域、空域和质量流；增强层以基本层作为起始点，对附加信息进行，从而在解码过程中重构更高层的质量、分辨率和时域层。通过解码基本层和相邻增强层，解码器能生成特定层的视频流。

## 2.2 mediasoup中的SVC
  得益于mediasoup良好的模块分化，SVC模块和上述的simulcast模块的实现类似。
```c++
class SvcConsumer : public RTC::Consumer, public RTC::RtpStreamSend::Listener
{
public:
	SvcConsumer(
	  const std::string& id,
	  const std::string& producerId,
	  RTC::Consumer::Listener* listener,
	  json& data);
	~SvcConsumer() override;

public:
	void FillJson(json& jsonObject) const override;
	void FillJsonStats(json& jsonArray) const override;
	void FillJsonScore(json& jsonObject) const override;
	void HandleRequest(Channel::ChannelRequest* request) override;
	RTC::Consumer::Layers GetPreferredLayers() const override
	{
		RTC::Consumer::Layers layers;

		layers.spatial  = this->preferredSpatialLayer;
		layers.temporal = this->preferredTemporalLayer;

		return layers;
	}
	bool IsActive() const override
	{
		// clang-format off
		return (
			RTC::Consumer::IsActive() &&
			this->producerRtpStream &&
			(this->producerRtpStream->GetScore() > 0u || this->producerRtpStream->HasDtx())
		);
		// clang-format on
	}
	void ProducerRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc) override;
	void ProducerNewRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc) override;
	void ProducerRtpStreamScore(RTC::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
	void ProducerRtcpSenderReport(RTC::RtpStream* rtpStream, bool first) override;
	uint8_t GetBitratePriority() const override;
	uint32_t IncreaseLayer(uint32_t bitrate, bool considerLoss) override;
	void ApplyLayers() override;
	uint32_t GetDesiredBitrate() const override;
	void SendRtpPacket(RTC::RtpPacket* packet) override;
	void GetRtcp(RTC::RTCP::CompoundPacket* packet, RTC::RtpStreamSend* rtpStream, uint64_t nowMs) override;
	std::vector<RTC::RtpStreamSend*> GetRtpStreams() override
	{
		return this->rtpStreams;
	}
	void NeedWorstRemoteFractionLost(uint32_t mappedSsrc, uint8_t& worstRemoteFractionLost) override;
	void ReceiveNack(RTC::RTCP::FeedbackRtpNackPacket* nackPacket) override;
	void ReceiveKeyFrameRequest(RTC::RTCP::FeedbackPs::MessageType messageType, uint32_t ssrc) override;
	void ReceiveRtcpReceiverReport(RTC::RTCP::ReceiverReport* report) override;
	uint32_t GetTransmissionRate(uint64_t nowMs) override;
	float GetRtt() const override;

private:
	void UserOnTransportConnected() override;
	void UserOnTransportDisconnected() override;
	void UserOnPaused() override;
	void UserOnResumed() override;
	void CreateRtpStream();
	void RequestKeyFrame();
	void MayChangeLayers(bool force = false);
	bool RecalculateTargetLayers(int16_t& newTargetSpatialLayer, int16_t& newTargetTemporalLayer) const;
	void UpdateTargetLayers(int16_t newTargetSpatialLayer, int16_t newTargetTemporalLayer);
	void EmitScore() const;
	void EmitLayersChange() const;

	/* Pure virtual methods inherited from RtpStreamSend::Listener. */
public:
	void OnRtpStreamScore(RTC::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
	void OnRtpStreamRetransmitRtpPacket(RTC::RtpStreamSend* rtpStream, RTC::RtpPacket* packet) override;

private:
	// Allocated by this.
	RTC::RtpStreamSend* rtpStream{ nullptr };
	// Others.
	std::vector<RTC::RtpStreamSend*> rtpStreams;
	RTC::RtpStream* producerRtpStream{ nullptr };
	bool syncRequired{ false };
	RTC::SeqManager<uint16_t> rtpSeqManager;
	int16_t preferredSpatialLayer{ -1 };
	int16_t preferredTemporalLayer{ -1 };
	int16_t provisionalTargetSpatialLayer{ -1 };
	int16_t provisionalTargetTemporalLayer{ -1 };
	std::unique_ptr<RTC::Codecs::EncodingContext> encodingContext;
	uint64_t lastBweDowngradeAtMs{ 0u }; // Last time we moved to lower spatial layer due to BWE.
};
```
与simulcast类似的分层概念也应用到了svc模块中。

而SVC的producer会在创建的时候和simulcast一样，根据传入的rtpParameters来创建。随后走层级切换的逻辑来进行传输：
```c++
void SvcConsumer::ApplyLayers()
{
	MS_TRACE();

	MS_ASSERT(this->externallyManagedBitrate, "bitrate is not externally managed");
	MS_ASSERT(IsActive(), "should be active");

	auto provisionalTargetSpatialLayer  = this->provisionalTargetSpatialLayer;
	auto provisionalTargetTemporalLayer = this->provisionalTargetTemporalLayer;

	// Reset provisional target layers.
	this->provisionalTargetSpatialLayer  = -1;
	this->provisionalTargetTemporalLayer = -1;

	if (!IsActive())
		return;

	// clang-format off
	if (
		provisionalTargetSpatialLayer != this->encodingContext->GetTargetSpatialLayer() ||
		provisionalTargetTemporalLayer != this->encodingContext->GetTargetTemporalLayer()
	)
	// clang-format on
	{
		UpdateTargetLayers(provisionalTargetSpatialLayer, provisionalTargetTemporalLayer);

		// If this looks like a spatial layer downgrade due to BWE limitations, set member.
		// clang-format off
		if (
			this->rtpStream->GetActiveMs() > BweDowngradeMinActiveMs &&
			this->encodingContext->GetTargetSpatialLayer() < this->encodingContext->GetCurrentSpatialLayer() &&
			this->encodingContext->GetCurrentSpatialLayer() <= this->preferredSpatialLayer
		)
		// clang-format on
		{
			MS_DEBUG_DEV(
			  "possible target spatial layer downgrade (from %" PRIi16 " to %" PRIi16
			  ") due to BWE limitation",
			  this->encodingContext->GetCurrentSpatialLayer(),
			  this->encodingContext->GetTargetSpatialLayer());

			this->lastBweDowngradeAtMs = DepLibUV::GetTimeMs();
		}
	}
}

...

uint32_t SvcConsumer::IncreaseLayer(uint32_t bitrate, bool considerLoss)
{
	MS_TRACE();

	MS_ASSERT(this->externallyManagedBitrate, "bitrate is not externally managed");
	MS_ASSERT(IsActive(), "should be active");

	if (this->producerRtpStream->GetScore() == 0u)
		return 0u;

	// If already in the preferred layers, do nothing.
	// clang-format off
	if (
		this->provisionalTargetSpatialLayer == this->preferredSpatialLayer &&
		this->provisionalTargetTemporalLayer == this->preferredTemporalLayer
	)
	// clang-format on
	{
		return 0u;
	}

	uint32_t virtualBitrate;

	if (considerLoss)
	{
		// Calculate virtual available bitrate based on given bitrate and our
		// packet lost.
		auto lossPercentage = this->rtpStream->GetLossPercentage();

		if (lossPercentage < 2)
			virtualBitrate = 1.08 * bitrate;
		else if (lossPercentage > 10)
			virtualBitrate = (1 - 0.5 * (lossPercentage / 100)) * bitrate;
		else
			virtualBitrate = bitrate;
	}
	else
	{
		virtualBitrate = bitrate;
	}

	uint32_t requiredBitrate{ 0u };
	int16_t spatialLayer{ 0 };
	int16_t temporalLayer{ 0 };
	auto nowMs = DepLibUV::GetTimeMs();

	for (; spatialLayer < this->producerRtpStream->GetSpatialLayers(); ++spatialLayer)
	{
		// If this is higher than current spatial layer and we moved to to current spatial
		// layer due to BWE limitations, check how much it has elapsed since then.
		if (nowMs - this->lastBweDowngradeAtMs < BweDowngradeConservativeMs)
		{
			if (this->provisionalTargetSpatialLayer > -1 && spatialLayer > this->encodingContext->GetCurrentSpatialLayer())
			{
				MS_DEBUG_DEV(
				  "avoid upgrading to spatial layer %" PRIi16 " due to recent BWE downgrade", spatialLayer);

				goto done;
			}
		}

		// Ignore spatial layers lower than the one we already have.
		if (spatialLayer < this->provisionalTargetSpatialLayer)
			continue;

		temporalLayer = 0;

		// Check bitrate of every temporal layer.
		for (; temporalLayer < this->producerRtpStream->GetTemporalLayers(); ++temporalLayer)
		{
			// Ignore temporal layers lower than the one we already have (taking into account
			// the spatial layer too).
			// clang-format off
			if (
				spatialLayer == this->provisionalTargetSpatialLayer &&
				temporalLayer <= this->provisionalTargetTemporalLayer
			)
			// clang-format on
			{
				continue;
			}

			requiredBitrate =
			  this->producerRtpStream->GetLayerBitrate(nowMs, spatialLayer, temporalLayer);

			MS_DEBUG_DEV(
			  "testing layers %" PRIi16 ":%" PRIi16 " [virtual bitrate:%" PRIu32
			  ", required bitrate:%" PRIu32 "]",
			  spatialLayer,
			  temporalLayer,
			  virtualBitrate,
			  requiredBitrate);

			// If active layer, end iterations here. Otherwise move to next spatial layer.
			if (requiredBitrate)
				goto done;
			else
				break;
		}

		// If this is the preferred or higher spatial layer, take it and exit.
		if (spatialLayer >= this->preferredSpatialLayer)
			break;
	}

done:

	// No higher active layers found.
	if (!requiredBitrate)
		return 0u;

	// No luck.
	if (requiredBitrate > virtualBitrate)
		return 0u;

	// Set provisional layers.
	this->provisionalTargetSpatialLayer  = spatialLayer;
	this->provisionalTargetTemporalLayer = temporalLayer;

	MS_DEBUG_DEV(
	  "upgrading to layers %" PRIi16 ":%" PRIi16 " [virtual bitrate:%" PRIu32
	  ", required bitrate:%" PRIu32 "]",
	  this->provisionalTargetSpatialLayer,
	  this->provisionalTargetTemporalLayer,
	  virtualBitrate,
	  requiredBitrate);

	if (requiredBitrate <= bitrate)
		return requiredBitrate;
	else if (requiredBitrate <= virtualBitrate)
		return bitrate;
	else
		return requiredBitrate; // NOTE: This cannot happen.
}

void SvcConsumer::ApplyLayers()
{
	MS_TRACE();

	MS_ASSERT(this->externallyManagedBitrate, "bitrate is not externally managed");
	MS_ASSERT(IsActive(), "should be active");

	auto provisionalTargetSpatialLayer  = this->provisionalTargetSpatialLayer;
	auto provisionalTargetTemporalLayer = this->provisionalTargetTemporalLayer;

	// Reset provisional target layers.
	this->provisionalTargetSpatialLayer  = -1;
	this->provisionalTargetTemporalLayer = -1;

	if (!IsActive())
		return;

	// clang-format off
	if (
		provisionalTargetSpatialLayer != this->encodingContext->GetTargetSpatialLayer() ||
		provisionalTargetTemporalLayer != this->encodingContext->GetTargetTemporalLayer()
	)
	// clang-format on
	{
		UpdateTargetLayers(provisionalTargetSpatialLayer, provisionalTargetTemporalLayer);

		// If this looks like a spatial layer downgrade due to BWE limitations, set member.
		// clang-format off
		if (
			this->rtpStream->GetActiveMs() > BweDowngradeMinActiveMs &&
			this->encodingContext->GetTargetSpatialLayer() < this->encodingContext->GetCurrentSpatialLayer() &&
			this->encodingContext->GetCurrentSpatialLayer() <= this->preferredSpatialLayer
		)
		// clang-format on
		{
			MS_DEBUG_DEV(
			  "possible target spatial layer downgrade (from %" PRIi16 " to %" PRIi16
			  ") due to BWE limitation",
			  this->encodingContext->GetCurrentSpatialLayer(),
			  this->encodingContext->GetTargetSpatialLayer());

			this->lastBweDowngradeAtMs = DepLibUV::GetTimeMs();
		}
	}
}

```
# 三、结语
  本文只是对mediasoup的simulcast和SVC的层级进行简单的介绍，后面会对simulcast或者svc整体应用的过程（包括：创建、传输、拥塞控制交互进行分析）。

## 细节1：数据包的类型怎么区分？
  WebRTC的Rtp数据除了根据PT进行区分，还会使用扩展字段去做一些额外的信息携带。在mediasoup中，Rtp数据来到Producer后会立刻进入前处理函数——PreProcessRtpPacket——一旦是Video数据就会打上 frameMarking07、frameMarking 这两个扩展字段。
```c++
// src/RTC/Producer.cpp
...
	inline void Producer::PreProcessRtpPacket(RTC::RtpPacket* packet)
	{
		MS_TRACE();

		if (this->kind == RTC::Media::Kind::VIDEO)
		{
			// NOTE: Remove this once framemarking draft becomes RFC.
			packet->SetFrameMarking07ExtensionId(this->rtpHeaderExtensionIds.frameMarking07);
			packet->SetFrameMarkingExtensionId(this->rtpHeaderExtensionIds.frameMarking);
		}
	}
...
```
打完扩展字段的标识我们就可以在RtpStreamRecv这个位置给每个包创建payloadDescriptorHandler（载荷描述处理能力）。
```c++
// src/RTC/RtpStreamRecv.cpp
...
bool RtpStreamRecv::ReceivePacket(RTC::RtpPacket* packet)
{
	...
	
	// Process the packet at codec level.
	if (packet->GetPayloadType() == GetPayloadType())
		RTC::Codecs::Tools::ProcessRtpPacket(packet, GetMimeType());

	...
}
...

// include/RTC/Codecs/Tools.hpp
static void ProcessRtpPacket(RTC::RtpPacket* packet, const RTC::RtpCodecMimeType& mimeType)
{
	switch (mimeType.type)
	{
		case RTC::RtpCodecMimeType::Type::VIDEO:
		{
			switch (mimeType.subtype)
			{
				case RTC::RtpCodecMimeType::Subtype::VP8:
				{
					RTC::Codecs::VP8::ProcessRtpPacket(packet);
					break;
				}

				case RTC::RtpCodecMimeType::Subtype::VP9:
				{
					RTC::Codecs::VP9::ProcessRtpPacket(packet);
					break;
				}

				case RTC::RtpCodecMimeType::Subtype::H264:
				{
					RTC::Codecs::H264::ProcessRtpPacket(packet);
					break;
				}
				case RTC::RtpCodecMimeType::Subtype::H264_SVC:
				{
					RTC::Codecs::H264_SVC::ProcessRtpPacket(packet);
					break;
				}
				default:;
			}
		}

		default:;
	}
}
...
```

 然后会在——MangleRtpPacket——中打上所有需要用的拓展字段。（这里我觉得mediasoup这样的实现不好，因为Consumer和Producer完全独立分开的，原意是在入口处把支持的所有拓展头都先预留出位置，在发送时再判断是否有打上具体的内容，如果没有就不会被序列化。但是这样的实现导致Consumer和Producer耦合了，我们在后续独立开发Consumer的功能时不得不考虑连Producer一起修改）。
```c++
// src/RTC/Producer.cpp
inline bool Producer::MangleRtpPacket(RTC::RtpPacket* packet, RTC::RtpStreamRecv* rtpStream) const
{
	MS_TRACE();

	// Mangle the payload type.
	{
		uint8_t payloadType = packet->GetPayloadType();
		auto it             = this->rtpMapping.codecs.find(payloadType);
		if (it == this->rtpMapping.codecs.end())
		{
			MS_WARN_TAG(rtp, "unknown payload type [payloadType:%" PRIu8 "]", payloadType);
			return false;
		}

		uint8_t mappedPayloadType = it->second;
		packet->SetPayloadType(mappedPayloadType);
	}

	// Mangle the SSRC.
	{
		uint32_t mappedSsrc = this->mapRtpStreamMappedSsrc.at(rtpStream);
		packet->SetSsrc(mappedSsrc);
	}

	// Mangle RTP header extensions.
	{
		thread_local static uint8_t buffer[4096];
		thread_local static std::vector<RTC::RtpPacket::GenericExtension> extensions;

		// This happens just once.
		if (extensions.capacity() != 24)
			extensions.reserve(24);

		extensions.clear();

		uint8_t* extenValue;
		uint8_t extenLen;
		uint8_t* bufferPtr{ buffer };
		// Add urn:ietf:params:rtp-hdrext:sdes:mid.
		{
			extenLen = RTC::MidMaxLength;
			extensions.emplace_back(
			  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::MID), extenLen, bufferPtr);
			bufferPtr += extenLen;
		}

		if (this->kind == RTC::Media::Kind::AUDIO)
		{
			// Proxy urn:ietf:params:rtp-hdrext:ssrc-audio-level.
			extenValue = packet->GetExtension(this->rtpHeaderExtensionIds.ssrcAudioLevel, extenLen);

			if (extenValue)
			{
				std::memcpy(bufferPtr, extenValue, extenLen);

				extensions.emplace_back(
				  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::SSRC_AUDIO_LEVEL),
				  extenLen,
				  bufferPtr);

				// Not needed since this is the latest added extension.
				// bufferPtr += extenLen;
			}
		}
		else if (this->kind == RTC::Media::Kind::VIDEO)
		{
			// Add http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time.
			{
				extenLen = 3u;

				// NOTE: Add value 0. The sending Transport will update it.
				uint32_t absSendTime{ 0u };
				Utils::Byte::Set3Bytes(bufferPtr, 0, absSendTime);
				extensions.emplace_back(
				  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::ABS_SEND_TIME), extenLen, bufferPtr);
				bufferPtr += extenLen;
			}

			// Add http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01.
			{
				extenLen = 2u;

				// NOTE: Add value 0. The sending Transport will update it.
				uint16_t wideSeqNumber{ 0u };
				Utils::Byte::Set2Bytes(bufferPtr, 0, wideSeqNumber);
				extensions.emplace_back(
				  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::TRANSPORT_WIDE_CC_01),
				  extenLen,
				  bufferPtr);
				bufferPtr += extenLen;
			}

			// NOTE: Remove this once framemarking draft becomes RFC.
			// Proxy http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07.
			extenValue = packet->GetExtension(this->rtpHeaderExtensionIds.frameMarking07, extenLen);

			if (extenValue)
			{
				std::memcpy(bufferPtr, extenValue, extenLen);
				extensions.emplace_back(
				  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::FRAME_MARKING_07),
				  extenLen,
				  bufferPtr);
				bufferPtr += extenLen;
			}

			// Proxy urn:ietf:params:rtp-hdrext:framemarking.
			extenValue = packet->GetExtension(this->rtpHeaderExtensionIds.frameMarking, extenLen);

			if (extenValue)
			{
				std::memcpy(bufferPtr, extenValue, extenLen);
				extensions.emplace_back(
				  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::FRAME_MARKING), extenLen, bufferPtr);
				bufferPtr += extenLen;
			}

			// Proxy urn:3gpp:video-orientation.
			extenValue = packet->GetExtension(this->rtpHeaderExtensionIds.videoOrientation, extenLen);

			if (extenValue)
			{
				std::memcpy(bufferPtr, extenValue, extenLen);
				extensions.emplace_back(
				  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::VIDEO_ORIENTATION),
				  extenLen,
				  bufferPtr);
				bufferPtr += extenLen;
			}

			// Proxy urn:ietf:params:rtp-hdrext:toffset.
			extenValue = packet->GetExtension(this->rtpHeaderExtensionIds.toffset, extenLen);
			if (extenValue)
			{
				std::memcpy(bufferPtr, extenValue, extenLen);
				extensions.emplace_back(
				  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::TOFFSET), extenLen, bufferPtr);
				bufferPtr += extenLen;
			}

			// Proxy http://www.webrtc.org/experiments/rtp-hdrext/abs-capture-time.
			extenValue = packet->GetExtension(this->rtpHeaderExtensionIds.absCaptureTime, extenLen);
			if (extenValue)
			{
				std::memcpy(bufferPtr, extenValue, extenLen);
				extensions.emplace_back(
				  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::ABS_CAPTURE_TIME),
				  extenLen,
				  bufferPtr);
				// Not needed since this is the latest added extension.
				// bufferPtr += extenLen;
			}
		}

		// Set the new extensions into the packet using One-Byte format.
		packet->SetExtensions(1, extensions);

		// Assign mediasoup RTP header extension ids (just those that mediasoup may
		// be interested in after passing it to the Router).
		packet->SetMidExtensionId(static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::MID));
		packet->SetAbsSendTimeExtensionId(
		  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::ABS_SEND_TIME));
		packet->SetTransportWideCc01ExtensionId(
		  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::TRANSPORT_WIDE_CC_01));
		// NOTE: Remove this once framemarking draft becomes RFC.
		packet->SetFrameMarking07ExtensionId(
		  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::FRAME_MARKING_07));
		packet->SetFrameMarkingExtensionId(
		  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::FRAME_MARKING));
		packet->SetSsrcAudioLevelExtensionId(
		  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::SSRC_AUDIO_LEVEL));
		packet->SetVideoOrientationExtensionId(
		  static_cast<uint8_t>(RTC::RtpHeaderExtensionUri::Type::VIDEO_ORIENTATION));
	}

	return true;
}

...
```
Producer处理完后调用自己的观察者上抛回Transport，而Transport管理了多个Consumer。在创建Consumer的时候已经根据拉流的信令来确定了SvcConsumer，于是Consumer中的Codecs就挂载上了H264-SVC的Codecs。Transport依次遍历所有的Consumer将数据下发到Consumer的SendRtpPacket函数，在这个函数中就会根据Codecs去识别payload的类型了。

```c++
// src/RTC/SvcConsumer.cpp	

	void SvcConsumer::SendRtpPacket(RTC::RtpPacket* packet)
	{
	
	...
	
		bool marker{ false };
		bool origMarker = packet->HasMarker();
		if (!packet->ProcessPayload(this->encodingContext.get(), marker))
		{
			this->rtpSeqManager.Drop(packet->GetSequenceNumber());
			return;
		}
		
	...
	
	}

```

## 细节2：GCC怎么与Consumer关联？
  GCC在mediasoup中是由TransportCongestionControlClient、TransportCongestionControlSever两个类来进行集成的，大家可以参考一下我这篇文章。这两个类都是在Transport这一层，大家可以发现Transport的概念更多的倾向于一个终端对一个Transport。怎么理解呢？我们一个Transport中会存在多个Consumer，而每个Consumer都可以理解为一条对应的流。
  方便理解，咱们以腾讯会议为例。一个主持人可能需要使用屏幕分享、视频交互、语音交互三条数据流，那么很合理的是一个用户如果需要同时看这三条流就需要拉三路。也就是说，一个用户（终端）需要同时拉三条流或者更多（多人会议）。但是对于服务端来说与该用户传输数据的网络是同一个，那么顺理成章的带宽估计就必须对那三条数据同时进行估计。

![](image0.png)

所以mediasoup在Transport级别创建了TransportCongestionControlClient类。但是这个带宽估计是需要结合每个数据包的TCC拓展字段来统计Feedback的，那么每个数据包就都需要在发送给下行之前打上TCC的拓展头下行才能根据拓展头字段计算delta来进行到达记录。上文介绍到在Producer中已经为所有支持的拓展头都预留了位置，具体写入内容的地方在Transport发送的地方（经过Consumer上抛回来）。

```c++
// src/RTC/Transport.cpp

...

// 里边这个宏 ENABLE_RTC_SENDER_BANDWIDTH_ESTIMATOR 是mediasoup自己实现的一个简单的带宽估计，一般是关闭的
// tccClient是使用webrtc内部的带宽估计集成的

	inline void Transport::OnConsumerSendRtpPacket(RTC::Consumer* consumer, RTC::RtpPacket* packet)
	{
		MS_TRACE();

		// Update abs-send-time if present.
		packet->UpdateAbsSendTime(DepLibUV::GetTimeMs());

		// Update transport wide sequence number if present.
		// clang-format off
		if (
			this->tccClient &&
			this->tccClient->GetBweType() == RTC::BweType::TRANSPORT_CC &&
			packet->UpdateTransportWideCc01(this->transportWideCcSeq + 1)
		)
		// clang-format on
		{
			this->transportWideCcSeq++;

			auto* tccClient = this->tccClient;
			webrtc::RtpPacketSendInfo packetInfo;

			packetInfo.ssrc                      = packet->GetSsrc();
			packetInfo.transport_sequence_number = this->transportWideCcSeq;
			packetInfo.has_rtp_sequence_number   = true;
			packetInfo.rtp_sequence_number       = packet->GetSequenceNumber();
			packetInfo.length                    = packet->GetSize();
			packetInfo.pacing_info               = this->tccClient->GetPacingInfo();

			// Indicate the pacer (and prober) that a packet is to be sent.
			this->tccClient->InsertPacket(packetInfo);

#ifdef ENABLE_RTC_SENDER_BANDWIDTH_ESTIMATOR
			auto* senderBwe = this->senderBwe;
			RTC::SenderBandwidthEstimator::SentInfo sentInfo;

			sentInfo.wideSeq     = this->transportWideCcSeq;
			sentInfo.size        = packet->GetSize();
			sentInfo.sendingAtMs = DepLibUV::GetTimeMs();

			auto* cb = new onSendCallback(
			  [tccClient, &packetInfo, senderBwe, &sentInfo](bool sent)
			  {
				  if (sent)
				  {
					  tccClient->PacketSent(packetInfo, DepLibUV::GetTimeMsInt64());

					  sentInfo.sentAtMs = DepLibUV::GetTimeMs();

					  senderBwe->RtpPacketSent(sentInfo);
				  }
			  });

			SendRtpPacket(consumer, packet, cb);
#else
			const auto* cb = new onSendCallback(
			  [tccClient, &packetInfo](bool sent)
			  {
				  if (sent)
					  tccClient->PacketSent(packetInfo, DepLibUV::GetTimeMsInt64());
			  });

			SendRtpPacket(consumer, packet, cb);
#endif
		}
		else
		{
			SendRtpPacket(consumer, packet);
		}

		this->sendRtpTransmission.Update(packet);
	}

...


```

上面的流程走完之后我们发现它内部还有个细节，就是在打拓展字段时，mediasoup代码直接写死了不让audio参与带宽估计，因此音频包都不会打上TCC的拓展字段。其实音频参与带宽估计在webrtc内部是支持的，这样可以提供更加准确的带宽估计值。
  先看层切换的函数 DistributeAvailableOutgoingBitrate，调用它的地方有三处。
```c++
void Transport::DistributeAvailableOutgoingBitrate()
{
	MS_TRACE();
	MS_ASSERT(this->tccClient, "no TransportCongestionClient");

	std::multimap<uint8_t, RTC::Consumer*> multimapPriorityConsumer;

	// Fill the map with Consumers and their priority (if > 0).
	for (auto& kv : this->mapConsumers)
	{
		auto* consumer = kv.second;
		auto priority  = consumer->GetBitratePriority();

		if (priority > 0u)
			multimapPriorityConsumer.emplace(priority, consumer);
	}

	// Nobody wants bitrate. Exit.
	if (multimapPriorityConsumer.empty())
		return;

	bool baseAllocation       = true;
	uint32_t availableBitrate = this->tccClient->GetAvailableBitrate();

	this->tccClient->RescheduleNextAvailableBitrateEvent();

	MS_DEBUG_DEV("before layer-by-layer iterations [availableBitrate:%" PRIu32 "]", availableBitrate);

	// Redistribute the available bitrate by allowing Consumers to increase
	// layer by layer. Initially try to spread the bitrate across all
	// consumers. Then allocate the excess bitrate to Consumers starting
	// with the highest priorty.
	while (availableBitrate > 0u)
	{
		auto previousAvailableBitrate = availableBitrate;

		for (auto it = multimapPriorityConsumer.rbegin(); it != multimapPriorityConsumer.rend(); ++it)
		{
			auto priority  = it->first;
			auto* consumer = it->second;
			auto bweType   = this->tccClient->GetBweType();

			for (uint8_t i{ 1u }; i <= (baseAllocation ? 1u : priority); ++i)
			{
				uint32_t usedBitrate{ 0u };

				switch (bweType)
				{
					case RTC::BweType::TRANSPORT_CC:
						usedBitrate = consumer->IncreaseLayer(availableBitrate, /*considerLoss*/ false);
						break;
					case RTC::BweType::REMB:
						usedBitrate = consumer->IncreaseLayer(availableBitrate, /*considerLoss*/ true);
						break;
				}

				MS_ASSERT(usedBitrate <= availableBitrate, "Consumer used more layer bitrate than given");

				availableBitrate -= usedBitrate;

				// Exit the loop fast if used bitrate is 0.
				if (usedBitrate == 0u)
					break;
			}
		}

		// If no Consumer used bitrate, exit the loop.
		if (availableBitrate == previousAvailableBitrate)
			break;

		baseAllocation = false;
	}

	MS_DEBUG_DEV("after layer-by-layer iterations [availableBitrate:%" PRIu32 "]", availableBitrate);

	// Finally instruct Consumers to apply their computed layers.
	for (auto it = multimapPriorityConsumer.rbegin(); it != multimapPriorityConsumer.rend(); ++it)
	{
		auto* consumer = it->second;
		consumer->ApplyLayers();
	}
}

```
 第一个触发方式：OnTargetTransferRate函数，它是WebRTC内部提供了一个接口。在TransportCongestionControlClient中也有这个函数。底层GCC接收到的delta数量足够计算时就会触发这个函数，或者其中定时触发也会触发该函数。
```c++
// src/RTC/Transport.cpp

...

void TransportCongestionControlClient::OnTargetTransferRate(webrtc::TargetTransferRate targetTransferRate)
{
	...
	MayEmitAvailableBitrateEvent(previousAvailableBitrate);
}

...

void TransportCongestionControlClient::MayEmitAvailableBitrateEvent(uint32_t previousAvailableBitrate)
{

...
	if (notify)
	{
		MS_DEBUG_DEV(
		  "notifying the listener with new available bitrate:%" PRIu32,
		  this->bitrates.availableBitrate);

		this->lastAvailableBitrateEventAtMs = nowMs;
		this->listener->OnTransportCongestionControlClientBitrates(this, this->bitrates);
	}
...
}

...

inline void Transport::OnTransportCongestionControlClientBitrates(
  RTC::TransportCongestionControlClient* /*tccClient*/,
  RTC::TransportCongestionControlClient::Bitrates& bitrates)
{

...
	DistributeAvailableOutgoingBitrate();
...

}


```

第二个触发方式：OnConsumerNeedBitrateChange函数，它是在SvcConsumer中当有主动切换层的请求、流出现异常变化时才会触发。
```c++
	inline void Transport::OnConsumerNeedBitrateChange(RTC::Consumer* /*consumer*/)
	{
	...
		DistributeAvailableOutgoingBitrate();
	...
	}

```

第三个触发方式：OnConsumerNeedZeroBitrate函数，这个函数则是在用户进行了暂停推流的操作时进行的。
```c++
	inline void Transport::OnConsumerNeedZeroBitrate(RTC::Consumer* /*consumer*/)
	{
		
	...

		DistributeAvailableOutgoingBitrate();
	
	...
	
	}
```
上面的内容实现了从带宽估计值传输到层筛选部分的逻辑，具体的层是怎么筛选的呢？所有的逻辑都在SvcConsumer中实现了。大值的方式就是：Producer统计所有分层的码率大小，然后再SvcConsumer中根据每个分层的大小去分配Svc足够使用哪一层，而且先到先得（除非设置了关键流的标志），大值的逻辑可以看看SvcConsumer简介那篇文章。

## 细节3：分层切换逻辑示意
[SVC分层切换逻辑示意](SVC%E5%88%86%E5%B1%82%E5%88%87%E6%8D%A2%E9%80%BB%E8%BE%91%E7%A4%BA%E6%84%8F.docx)

# 三、总结
   本文简单介绍了mediasoup中的SVC，同时给了一个使用H264-SVC的案例——代码就不展示了。整个测试的效果是：
    1.带宽场景，最低抵抗600kbps的带宽限制（跟T0层、分辨率有关，做的更小可以抵抗更低）；
    2.混合场景，可抵抗10%随机丢包+700kbps带宽限制。
  大家可以一起讨论一下方案的问题，我们共同进步。