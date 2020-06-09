http://www.rtcbits.com/2017/01/bandwidth-estimation-in-webrtc-and-new.html

Bandwidth Estimation in WebRTC (and the new Sender Side BWE)

Bandwidth estimation is probably the most critical component in the video engine of WebRTC. The bandwidth estimation (BWE) module is responsible for deciding how much video* traffic you can send without congesting the network to prevent degradation of the video quality.

In the past bandwidth estimation algorithms used to be very rudimentary and based mostly on packet loss. Basically we used to start increasing slowly the video bitrate until we detected packets being lost.  To detect the packet loss you use the standard RTCP feedback mechanisms where the receiver side reports packet loss periodically using RTCP Receiver Report (RR) messages.

Modern bandwidth estimation algorithms are more advanced and try to detect congestion before it is bad enough to make routers discard packets. These algorithms predicts congestion analyzing the delay between packets. The idea is that when you start having some congestion, the buffers in the routers will start filling and the delay will be more variable. Some popular examples of these algorithms are Google Congestion Control (the one used in WebRTC), SCReAM and SPROUT.  If you want to read more about the history and status of congestion control standards you can read this very interesting post from Randell Jesup.

From the very beginning of WebRTC, the media engine (that is built by Google but included in both Chrome and Firefox) was based on the concept of remote bandwidth estimation. As explained before the receiver of the traffic analyzes the inter-packet delay and generates an estimation of the available bandwidth that is reported back to the sender using RTCP messages with a new message type that was defined for this purpose: REMB. Another detail of WebRTC implementation is that the sender will use not only this bandwidth estimation received in the REMB packet but also the packet loss feedback to decide the final value of the target video bitrate to be sent.

Sender pseudocode (send_side_bandwidth_estimation.cc):
  onFeedbackFromReceiver(lossRate):
    if (lossRate < 2%) video_bitrate *= 1.08
    if (lossRate > 10%) video_bitrate *= (1 - 0.5*lossRate)
    if (video_bitrate > bwe) video_bitrate = bwe;

The nice consequence of that implementation is that it reduces bitrate quickly when overuse is detected while slowly increasing bitrate when no congestion is detected.

But in recent versions of Chrome this has changed and now the whole bandwidth estimation logic happens in the sender side.   The basic detection of congestion is not very different from how it was before and the sender needs delay information from the receiver side in order to be able to estimate the available bandwidth.  This is accomplished with two new mechanisms/protocols:

1. Transport wide sequence numbers header extension:   All the video RTP packets carry an extra 4 bytes in the header to include a sequence number.    This is negotiated in the SDP with the following line:

a=extmap:5 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01

Note: the idea of this new sequence number is to be able to use a single counter for both audio and video but Chrome still doesn't use it for audio, so I think it is not very useful yet.


2. Transport Feedback: The receiver side sends periodic feedback to the media sender with information  about the packets received and the the delay between them.  To do this the receiver uses a new RTCP Packet (Transport Feedback).  This feature is negotiated in the SDP with this line including the new RTCP feedback message:

a=rtcp-fb:100 transport-cc

When looking at how is this transport feedback packet looks like it is funny to realize that there is a specification with Google's proposal and an official standardization proposal but the only source of truth is the real implementation in the source code.

This RTCP feedback is sent every 100 msecs by default but it is dynamically adapted to use 5% of the available bandwidth (min value is 50 msecs and max is 250 msecs).

The format of this new RTCP packet is very concise to minimize the size, including grouping packets in chunks, storing numbers as base + diff or reducing granularity to 0.25 msec intervals. I did a simple test and even with these improvements it was still using 16Kbps sending feedback packets every 50 msecs (Figure 2).


Figure 2: Bandwidth used in RTCP Transport Feedback Messages
You can take a look at the implementation in remote_estimator_proxy.h (generating the packets) and transport_feedback.cc (serialization).

What is good about sender side bandwidth estimation? The theory as explained by Google is that this way all the decision logic is in a single place (the sender) and that it makes it possible to test new algorithms easily because you don't depend on both endpoints. Honestly given that browsers auto update I don't see the big advantage of this point but it is certainly cleaner even if it is more expensive in bandwidth usage. The other advantage is that the sender knows the type of traffic he is sending and can use a different algorithm when sending normal video than when doing a screencast for example.

Are we affected? If you are building a media server that requires bandwidth estimation for anything (for example to decide the quality to forward when using simulcast) you will need to upgrade your implementation at some point. The good news is that Chrome will have to support the old mechanism (REMB) for a while, at least until Firefox includes support for it.   But REMB probably won't get more improvements and it is more likely to have bugs now so probably not a good idea to postpone the change much.

Is sender side bwe really better? I did a quick test (this is the test page where you can try one or the other changing a boolean) with both algorithms in Chrome (old REMB vs new Transport Feedback) and the new one performed way better at least regarding ramp-up time at the beginning of the connection (see figures below).   I don't think there is a technical reason for that apart from the fact that Google is now focused on the new one and not the old one and all the new improvements are probably only in the new algorithm.   Apparently there is something in the new code to handle in a special way the bwe during the first 2 seconds but I didn't investigate it much.



Who is working on this and what is the status?  Sender side bandwidth estimation is the default in Chrome 55 but this is still work in progress and we should expect many changes.   The official standardization is happening in IETF in the RMCAT group but most of the implementation available in Chrome is Google's own version of the in-progress specifications for algorithms and feedback protocols.

* Chrome is plannin
