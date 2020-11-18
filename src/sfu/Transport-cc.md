
####BWE and Congestion Control


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
a=extmap:5 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01iiw
1
Transport Feedback The
receiver sends feedback periodically to the media sender, providing information about the received data packets and the delay between them. The feedback information is fed back to the sender through RTCP-Transport-FB. Negotiate whether to enable through SDP
a=rtcp-fb:100 transport-cc

##Regarding the frequency of TransportFeedbacks that mediasoup must send to the remote sender:

In Bandwidth Estimation in WebRTC (and the new Sender Side BWE)
http://www.rtcbits.com/2017/01/bandwidth-estimation-in-webrtc-and-new.html

This RTCP feedback is sent every 100 msecs by default but it is dynamically adapted to use 5% of the available bandwidth (min value is 50 msecs and max is 250 msecs).


## Receiver Estimated Max Bitrate (REMB)

REMB stands for Receiver Estimated Maximum Bitrate. It is a RTCP message used to provide bandwidth estimation in order to avoid creating congestion in the network.

This RTCP message includes a field to convey the total estimated available bitrate on the path to the receiving side of this RTP session (in mantissa + exponent format). Even if it is defined as the total available bitrate, the sender typically uses it to configure the maximum bitrate of the video encoding.

In addition to be used from an endpoint to notify the available bandwidth in the network, it has also been used by media servers to limit the amount of bitrate the sender is allowed to send.

To provide a better estimation, REMB is usually used in combination with the abs-send-time header extension because providing accurate timing information is critical for the accuracy of the REMB value calculation.

This RTCP message defined in draft-alvestrand-rmcat-remb-03 was never fully standardized but is supported by all the WebRTC browser implementations, although in case of Chrome it is deprecated in favor of the new sender side bandwidth estimation based on RTCP Transport Feedback messages. The support for this message is negotiated in the Offer/Answer SDP Exchange.

2.1. Semantics
This feedback message is used to notify a sender of multiple media streams over the same RTP session of the total estimated available bit rate on the path to the receiving side of this RTP session.

Within the common packet header for feedback messages (as defined in section 6.1 of [RFC4585]), the "SSRC of packet sender" field indicates the source of the notification. The "SSRC of media source" is not used and SHALL be set to 0. This usage of the value zero is also done in other RFCs.

The reception of a REMB message by a media sender conforming to this specification SHALL result in the total bit rate sent on the RTP session this message applies to being equal to or lower than the bit rate in this message. The new bit rate constraint should be applied as fast as reasonable. The sender is free to apply additional bandwidth restrictions based on its own restrictions and estimates.

2.2. Message format
This document describes a message using the application specific payload type. This is suitable for experimentation; upon standardization, a specific type can be assigned for the purpose.

The message is an RTCP message with payload type 206. RFC 3550 [RFC3550] defines the range, RFC 4585 defines the specific PT value 206 and the FMT value 15.

 0                   1                   2                   3               
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P| FMT=15  |   PT=206      |             length            |                               
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                  SSRC of packet sender                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                  SSRC of media source                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Unique identifier 'R' 'E' 'M' 'B'                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Num SSRC     | BR Exp    |  BR Mantissa                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   SSRC feedback                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  ...                                                          |

The fields V, P, SSRC, and length are defined in the RTP specification [2], the respective meaning being summarized below:

version (V): (2 bits):
This field identifies the RTP version. The current version is 2.
padding (P) (1 bit):
If set, the padding bit indicates that the packet contains additional padding octets at the end that are not part of the control information but are included in the length field. Always 0.
Feedback message type (FMT) (5 bits):
This field identifies the type of the FB message and is interpreted relative to the type (transport layer, payload- specific, or application layer feedback). Always 15, application layer feedback message. RFC 4585 section 6.4.
Payload type (PT) (8 bits):
This is the RTCP packet type that identifies the packet as being an RTCP FB message. Always PSFB (206), Payload-specific FB message. RFC 4585 section 6.4.
Length (16 bits):
The length of this packet in 32-bit words minus one, including the header and any padding. This is in line with the definition of the length field used in RTCP sender and receiver reports [3]. RFC 4585 section 6.4.
SSRC of packet sender (32 bits):
The synchronization source identifier for the originator of this packet. RFC 4585 section 6.4.
SSRC of media source (32 bits):
Always 0; this is the same convention as in [RFC5104] section 4.2.2.2 (TMMBN).
Unique identifier (32 bits):
Always ‘R’ ‘E’ ‘M’ ‘B’ (4 ASCII characters).
Num SSRC (8 bits):
Number of SSRCs in this message.
BR Exp (6 bits):
The exponential scaling of the mantissa for the maximum total media bit rate value, ignoring all packet overhead. The value is an unsigned integer [0..63], as in RFC 5104 section 4.2.2.1.
BR Mantissa (18 bits):
The mantissa of the maximum total media bit rate (ignoring all packet overhead) that the sender of the REMB estimates. The BR is the estimate of the traveled path for the SSRCs reported in this message. The value is an unsigned integer in number of bits per second.
SSRC feedback (32 bits)
Consists of one or more SSRC entries which this feedback message applies to.
2.3. Signaling of use of this extension
We negotiate use of the message in SDP using a header extension according to RFC 4585 section 4.2, with the value "goog-remb":

a=rtcp-fb:<payload type> goog-remb

3. Absolute Send Time
Google has found that there are issues with relative send time offset when packets are relayed at nodes that are not the source of the RTP clock; it is hard to generate accurate offsets when you have to regenerate the base clock from the incoming packets before you can figure out what time to match; also, using signals from multiple flows becomes impossible unless the timestamps come from a common clock.

The Absolute Send Time extension is used to stamp RTP packets with a timestamp showing the departure time from the system that put this packet on the wire (or as close to this as we can manage).

[RFC5285] is used:

Name: "Absolute Sender Time" ; "RTP Header Extension for Absolute Sender Time".
Formal name: "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time".
Wire format: 1-byte extension, 3 bytes of data. total 4 bytes extra per packet (plus shared 4 bytes for all extensions present: 2 byte magic word 0xBEDE, 2 byte # of extensions).
Encoding: Timestamp is in seconds, 24 bit 6.18 fixed point, yielding 64s wraparound and 3.8us resolution (one increment for each 477 bytes going out on a 1Gbps interface).
Relation to NTP timestamps: abs_send_time_24 = (ntp_timestamp_64 >> 14) & 0x00ffffff ; NTP timestamp is the number of seconds since the epoch, in 32.32 bit fixed point format.
Notes: Packets are time stamped when going out, preferably close to metal. Intermediate RTP relays (RTP entities possibly altering the relative timing of packets in the stream) should remove the extension or overwrite its value with its own timestamp.
When signalled in SDP, the standard mechanism for RTCP extensions

a=extmap:3 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time









###Simulcast
Simulcast only good when sender is Desktop. For Mobile to mobile simulcast is very bad.


Simulcast is a technique by which a WebRTC client encodes the same video stream twice in different resolutions and bitrates and sending these to a router who then decides who receives which of the streams.

Simulcast is closely related to SVC, where a single encoded video stream can be layered and each participant receives only the layers that he can process.