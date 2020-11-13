### Transprot-CC vs REMB

#Introduction to Transport-CC
Webrtc has two congestion control feedback methods, one is REMB and the other is Transport-CC. Transport-CC (Transport-wide Congestion Control). REMB is the code rate feedback based on the packet loss rate at the receiving end , and Transport-CC is the code rate control based on the delay at the transmitting end . Of course, both types of feedback packets are fed back from the receiving end to the transmitting end.

If you want to enable this algorithm, the sender needs to add the transport-wide sequence number field in the RTP extension header, and the receiver RTCP sends the TransportFeedback message.

#Transport-wide-cc vs  transport-wide-cc-02
Implement Transport-wide Congestion ControlT
Transport-wide CC
Transport-wide CC 2 (which may  send a feedback packet for every packet instead of sending them on an interval).

Transport-Wide Congestion Control
This RTP header extension is an extended version of the extension defined in https://tools.ietf.org/html/draft-holmer-rmcat-transport-wide-cc-extensions-01

Name: “Transport-wide congenstion control 02”

Formal name: http://www.webrtc.org/experiments/rtp-hdrext/transport-wide-cc-02


The original extension defines a transport-wide sequence number that is used in feedback packets for congestion control. The original implementation sends these feedback packets at a periodic interval. The extended version presented here has two changes compared to the original version:

Feedback is sent only on request by the sender, therefore, the extension has two optional bytes that signals that a feedback packet is requested.
The sender determines if timing information should be included or not in the feedback packet. The original version always include timing information.
Contact kron@google.com or sprang@google.com for more info.

RTP header extension format
Data layout overview
Data layout of transport-wide sequence number 1-byte header + 2 bytes of data:

  0                   1                   2
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |  ID   | L=1   |transport-wide sequence number |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Data layout of transport-wide sequence number and optional feedback request 1-byte header + 4 bytes of data:

  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |  ID   | L=3   |transport-wide sequence number |T|  seq count  |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |seq count cont.|
 +-+-+-+-+-+-+-+-+
Data layout details
The data is written in the following order,

transport-wide sequence number (16-bit unsigned integer)
feedback request (optional) (16-bit unsigned integer)
If the extension contains two extra bytes for feedback request, this means that a feedback packet should be generated and sent immediately. The feedback request consists of a one-bit field giving the flag value T and a 15-bit field giving the sequence count as an unsigned number.
If the bit T is set the feedback packet must contain timing information.
seq count specifies how many packets of history that should be included in the feedback packet. If seq count is zero no feedback should be be generated, which is equivalent of sending the two-byte extension above. This is added as an option to allow for a fixed packet header size.


RTCPtransport-wide-cc-01 

enabling transport-wide-cc-01 exten, all TCC feedback packets received from Chrome have negative referenceTime. Our parsing:

<FeedbackRtpTransportPacket>
   base sequence         : 1
   packet status count   : 2
   reference time        : -4368470
   feedback packet count : 0
   size                  : 24
   <RunLengthChunk>
     status : SD
     count  : 2
   </RunLengthChunk>
   <Deltas>
     35ms
     17ms
   </Deltas>
   <PacketResults>
     seq:1, received:yes, receivedAt:-279582045
     seq:2, received:yes, receivedAt:-279582028
   </PacketResults>
 </FeedbackRtpTransportPacket>

https://trac.tools.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01.html

Transport wide cc is a kind of feedback mechanism

T=RTPFB 205
FMT=15 The receiver constructs TransportCC message rfc transport wide cc
Principle: After receiving the rtp packet, the receiving end stores the sequence number (reversal here) and the arrival timestamp, and then calculates the relative delay of each packet at regular intervals, and transmits it through transport wide cc. After receiving it, the receiving end uses some algorithms to calculate the delay and estimate the bandwidth.
Use of janus-gateway's transport wide cc as the receiving end
As the sender, janus did not see any processing of transport cc.

The first step is to check whether the SDP information in the offer contains transport wide cc information:

PT=RTPFB 205
FMT=15 The receiver constructs TransportCC message rfc transport wide cc
Principle: After receiving the rtp packet, the receiving end stores the sequence number (reversal here) and the arrival timestamp, and then calculates the relative delay of each packet at regular intervals, and transmits it through transport wide cc. After receiving it, the receiving end uses some algorithms to calculate the delay and estimate the bandwidth.
Use of janus-gateway's transport wide cc as the receiving end
As the sender, janus did not see any processing of transport cc.


Transport wide sequence numbers header extension
x All RTP packets add an additional header extension item, which is used to indicate the transmission sequence number. Use SDP to negotiate whether to open the extension
a=extmap:5 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
1
Transport Feedback The
receiver sends feedback periodically to the media sender, providing information about the received data packets and the delay between them. The feedback information is fed back to the sender through RTCP-Transport-FB. Negotiate whether to enable through SDP
a=rtcp-fb:100 transport-cc

##Regarding the frequency of TransportFeedbacks that mediasoup must send to the remote sender:

In Bandwidth Estimation in WebRTC (and the new Sender Side BWE)
http://www.rtcbits.com/2017/01/bandwidth-estimation-in-webrtc-and-new.html

This RTCP feedback is sent every 100 msecs by default but it is dynamically adapted to use 5% of the available bandwidth (min value is 50 msecs and max is 250 msecs).

