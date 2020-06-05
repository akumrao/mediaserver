

|   Streams| Resolution  |
| ------------ | ------------ |
| HD  | 280 x 720  |
| SD  |  640 x 360 |
|  CIF | 320 x 180  |


**The stream selection is based on the RTCP feedback packets sent by SFU, which can be either:**

- REMB packets, in cases where the sender's network bandwidth estimation is performed by SFU and sent back to the sender

- TransportCC packets, in cases where the sender's network bandwidth estimation is performed by the sender, based on TrasnportCC packet data

*FEC Forward Error Correction  and RED* 
When you use FEC we use RED to encapsulate the payload (in this case
VP8) and FEC. The whole stream will look like one RED stream
independent on the level of FEC protection. Hence when FEC is enabled
we will add the RED header to all packets even if FEC is currently not
used. FEC can be "on" for a fraction of frames (packets) or "on" for
all frames. This is determined by the importance of the frame and
packet (as an example the packets of a key frame is most important a
packet to an enhancement temporal layer is nice to have but not
important for the stream)

The only way to know if the packet is FEC or VP8 is to look direct
after the RTP header, where the RED header is located.

Since the stream look like a normal RTP stream where RED is the
payload type and all packets come in sequence without gaps there is no
issue to use SRTP.

*ULPFEC* stands for Uneven Level Protection Forward Error Correction
 XOR across multiple packets to generate this redundant information and be able to recover lost packets in the receiver side
 
 RTCP NACK mechanism to report missing RTP packets.
 Picture Loss Indication (PLI) to request a full keyframe from the remote party

FIR (Full Intra Request)
FIR when ecoder changes so request for Intra frame
PLI when decoder no more decode because of interframes loss so request key frame ( IDR)


at simulcast or setPropterties we can set
dtx
Only used for an RTCRtpSender whose kind is audio, this property indicates whether or not to use discontinuous transmission (a feature by which a phone is turned off or the microphone muted automatically in the absence of voice activity). The value is taken from the enumerated string type RTCDtxStatus.

**Chrome**

To enable simulcast/scalability support in Chrome, you have to munge the SDP Offer with the so-called SIM group to configure the number of resolutions to send (maximum of 3 if you are sending video in HD and a maximum of 1 for screen-sharing) and include a special flag in the SDP Answer (x-conference-flag) (see here and here for more information)..  One of the nicest parts of enabling simulcast in Chrome is that it automatically enables temporal scalability in VP8 as well (more info here).

**Firefox and standard methods**

More recently, standards and Firefox are also adding simulcast support by means of  a new protocol feature called RID. RID allows the  identification of the different simulcast streams in RTP packets. ORTC and latest WebRTC specification goes much further adding APIs to tune the parameters of the different simulcast qualities (WebRTC spec, ORTC spec).



Simulcast is one of the more interesting aspects of WebRTC for multiparty conferencing. In a nutshell, it means sending three different resolution (spatial scalability) and different frame rates (temporal scalability) at the same time. Oscar Divorra’s post contains the full details.

Usually, one needs a SFU to take advantage of simulcast. But there is a hack to make the effect visible between two browsers — or inside a single page. This is very helpful for single-page tests or fiddling with simulcast features, particular the ability to enable only certain spatial layers or to control the target bitrate of a particular stream.

The Playground
There are two variants of the playground, one for Chrome and another one for Firefox. The Chrome variant uses the same old SDP munging hack that we have first seen in Hangouts back in summer 2014. Firefox uses RTCRtpSender.setParameters  to enable simulcast. Neither is complying with the latest specification, but that has not stopped anyone from using it really.

Both variants show the video image, first the sender image and overall bitrate/frame rate graphs followed by the three different spatial streams, each with corresponding graphs for bitrate and framerate.



How bad is the hack required to accomplish this?
Since we don’t want to involve a server, we need to hack things. Fortunately there was a Chrome test which showed the idea. Both Chrome and Firefox use the RTP SSRC of a packet to route it to a certain mediastream. These SSRC can be found in the SDP offer:

- Original Chrome SDP
- simulcast SDP, Chrome
- type: offer, sdp: v=0
- o=- 7356021969196541917 2 IN IP4 127.0.0.1
- s=-
- t=0 0
- a=group:BUNDLE video
- a=msid-semantic: WMS 0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J
- m=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 102 124 127 123 125
- c=IN IP4 0.0.0.0
- a=rtcp:9 IN IP4 0.0.0.0
- a=ice-ufrag:cTTs
- a=ice-pwd:W9/g2uTwfb6UCRxfIoMkd5nV
- a=ice-options:trickle
- a=fingerprint:sha-256 01:09:17:BA:CD:91:FE:E0:24:24:86:5C:17:71:CC:37:61:CF:BA:D1:31:49:80:F1:BC:B8:B2:6F:8C:D5:39:F2
- a=setup:actpass
- a=mid:video
- a=extmap:2 urn:ietf:params:rtp-hdrext:toffset
- a=extmap:3 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
- a=extmap:4 urn:3gpp:video-orientation
- a=extmap:5 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
- a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
- a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
- a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
- a=sendrecv
- a=rtcp-mux
- a=rtcp-rsize
- a=rtpmap:96 VP8/90000
- a=rtcp-fb:96 goog-remb
- a=rtcp-fb:96 transport-cc
- a=rtcp-fb:96 ccm fir
- a=rtcp-fb:96 nack
- a=rtcp-fb:96 nack pli
- a=rtpmap:97 rtx/90000
- a=fmtp:97 apt=96
- a=rtpmap:98 VP9/90000
- a=rtcp-fb:98 goog-remb
- a=rtcp-fb:98 transport-cc
- a=rtcp-fb:98 ccm fir
- a=rtcp-fb:98 nack
- a=rtcp-fb:98 nack pli
- a=rtpmap:99 rtx/90000
- a=fmtp:99 apt=98
- a=rtpmap:100 H264/90000
- a=rtcp-fb:100 goog-remb
- a=rtcp-fb:100 transport-cc
- a=rtcp-fb:100 ccm fir
- a=rtcp-fb:100 nack
- a=rtcp-fb:100 nack pli
- a=fmtp:100 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f
- a=rtpmap:101 rtx/90000
- a=fmtp:101 apt=100
- a=rtpmap:102 H264/90000
- a=rtcp-fb:102 goog-remb
- a=rtcp-fb:102 transport-cc
- a=rtcp-fb:102 ccm fir
- a=rtcp-fb:102 nack
- a=rtcp-fb:102 nack pli
- a=fmtp:102 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f
- a=rtpmap:124 rtx/90000
- a=fmtp:124 apt=102
- a=rtpmap:127 red/90000
- a=rtpmap:123 rtx/90000
- a=fmtp:123 apt=127
- a=rtpmap:125 ulpfec/90000
- a=ssrc-group:FID 3818935445 664019814
- a=ssrc:3818935445 cname:/9xLVG5y0PJqxacG
- a=ssrc:3818935445 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:3818935445 mslabel:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J
- a=ssrc:3818935445 label:d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:664019814 cname:/9xLVG5y0PJqxacG
- a=ssrc:664019814 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:3818935446 cname:/9xLVG5y0PJqxacG
- a=ssrc:3818935446 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:3818935447 cname:/9xLVG5y0PJqxacG
- a=ssrc:3818935447 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:3818935448 cname:/9xLVG5y0PJqxacG
- a=ssrc:3818935448 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:3818935449 cname:/9xLVG5y0PJqxacG
- a=ssrc:3818935449 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc-group:FID 3818935446 3818935447
- a=ssrc-group:FID 3818935448 3818935449
- a=ssrc-group:SIM 3818935445 3818935446 3818935448


- type: offer, sdp: v=0
- o=- 7356021969196541917 2 IN IP4 127.0.0.1
- s=-
- t=0 0
- a=group:BUNDLE video
- a=msid-semantic: WMS 0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J
- m=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 102 124 127 123 125
- c=IN IP4 0.0.0.0
- a=rtcp:9 IN IP4 0.0.0.0
- a=ice-ufrag:cTTs
- a=ice-pwd:W9/g2uTwfb6UCRxfIoMkd5nV
- a=ice-options:trickle
- a=fingerprint:sha-256 01:09:17:BA:CD:91:FE:E0:24:24:86:5C:17:71:CC:37:61:CF:BA:D1:31:49:80:F1:BC:B8:B2:6F:8C:D5:39:F2
- a=setup:actpass
- a=mid:video
- a=extmap:2 urn:ietf:params:rtp-hdrext:toffset
- a=extmap:3 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
- a=extmap:4 urn:3gpp:video-orientation
- a=extmap:5 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
- a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
- a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
- a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
- a=sendrecv
- a=rtcp-mux
- a=rtcp-rsize
- a=rtpmap:96 VP8/90000
- a=rtcp-fb:96 goog-remb 
- a=rtcp-fb:96 transport-cc
- a=rtcp-fb:96 ccm fir
- a=rtcp-fb:96 nack
- a=rtcp-fb:96 nack pli
- a=rtpmap:97 rtx/90000
- a=fmtp:97 apt=96
- a=rtpmap:98 VP9/90000
- a=rtcp-fb:98 goog-remb
- a=rtcp-fb:98 transport-cc
- a=rtcp-fb:98 ccm fir
- a=rtcp-fb:98 nack
- a=rtcp-fb:98 nack pli
- a=rtpmap:99 rtx/90000
- a=fmtp:99 apt=98
- a=rtpmap:100 H264/90000
- a=rtcp-fb:100 goog-remb
- a=rtcp-fb:100 transport-cc
- a=rtcp-fb:100 ccm fir
- a=rtcp-fb:100 nack
- a=rtcp-fb:100 nack pli
- a=fmtp:100 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f
- a=rtpmap:101 rtx/90000
- a=fmtp:101 apt=100
- a=rtpmap:102 H264/90000
- a=rtcp-fb:102 goog-remb
- a=rtcp-fb:102 transport-cc
- a=rtcp-fb:102 ccm fir
- a=rtcp-fb:102 nack
- a=rtcp-fb:102 nack pli
- a=fmtp:102 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f
- a=rtpmap:124 rtx/90000
- a=fmtp:124 apt=102
- a=rtpmap:127 red/90000
- a=rtpmap:123 rtx/90000
- a=fmtp:123 apt=127
- a=rtpmap:125 ulpfec/90000
- a=ssrc-group:FID 3818935445 664019814
- a=ssrc:3818935445 cname:/9xLVG5y0PJqxacG
- a=ssrc:3818935445 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:3818935445 mslabel:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J
- a=ssrc:3818935445 label:d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:664019814 cname:/9xLVG5y0PJqxacG
- a=ssrc:664019814 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:3818935446 cname:/9xLVG5y0PJqxacG
- a=ssrc:3818935446 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:3818935447 cname:/9xLVG5y0PJqxacG
- a=ssrc:3818935447 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:3818935448 cname:/9xLVG5y0PJqxacG
- a=ssrc:3818935448 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc:3818935449 cname:/9xLVG5y0PJqxacG
- a=ssrc:3818935449 msid:0AQNo1bnpzlGvB7aE2InJRz85M9lmId7Es9J d1c9f46b-0328-4ef1-9c31-c1b279ac554a
- a=ssrc-group:FID 3818935446 3818935447
- a=ssrc-group:FID 3818935448 3818935449
- a=ssrc-group:SIM 3818935445 3818935446 3818935448
- As in 2014, the important thing here are the multiple a=ssrc:  lines as well as the a=ssrc-group:SIM.

- Original Firefox SDP
- Firefox simulcast SDP
- v=0
- o=mozilla...THIS_IS_SDPARTA-61.0.1 2246157997147315987 0 IN IP4 0.0.0.0
- s=-
- t=0 0
- a=sendrecv
- a=fingerprint:sha-256 00:3E:DC:02:92:60:97:0D:D8:F7:7F:E9:AD:41:46:CD:B5:FC:33:35:3F:5C:C4:BC:CD:85:17:96:F8:D6:14:57
- a=group:BUNDLE sdparta_0
- a=ice-options:trickle
- a=msid-semantic:WMS *
- m=video 40601 UDP/TLS/RTP/SAVPF 120 121 126 97
- c=IN IP4 192.168.1.230
- a=sendrecv
- a=end-of-candidates
- a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
- a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
- a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
- a=extmap:6/sendonly urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
- a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
- a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
- a=fmtp:120 max-fs=12288;max-fr=60
- a=fmtp:121 max-fs=12288;max-fr=60
- a=ice-pwd:f317cfe0aaa529381815e208a6cdec19
- a=ice-ufrag:07366c17
- a=mid:sdparta_0
- a=msid:{8de6968c-0ea9-4a19-aaa6-51355d4a5f4e} {9bdb3a96-19f3-4730-8459-e56db515b5da}
- a=rid:hi send
- a=rid:mid send
- a=rid:lo send
- a=rtcp-fb:120 nack
- a=rtcp-fb:120 nack pli
- a=rtcp-fb:120 ccm fir
- a=rtcp-fb:120 goog-remb
- a=rtcp-fb:121 nack
- a=rtcp-fb:121 nack pli
- a=rtcp-fb:121 ccm fir
- a=rtcp-fb:121 goog-remb
- a=rtcp-fb:126 nack
- a=rtcp-fb:126 nack pli
- a=rtcp-fb:126 ccm fir
- a=rtcp-fb:126 goog-remb
- a=rtcp-fb:97 nack
- a=rtcp-fb:97 nack pli
- a=rtcp-fb:97 ccm fir
- a=rtcp-fb:97 goog-remb
- a=rtcp-mux
- a=rtpmap:120 VP8/90000
- a=rtpmap:121 VP9/90000
- a=rtpmap:126 H264/90000
- a=rtpmap:97 H264/90000
- a=setup:actpass
- a=simulcast: send rid=hi;mid;lo
- a=ssrc:252082699 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}
- a=ssrc:2611961929 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}
- a=ssrc:1452646660 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}



- v=0
- o=mozilla...THIS_IS_SDPARTA-61.0.1 2246157997147315987 0 IN IP4 0.0.0.0
- s=-
- t=0 0
- a=sendrecv
- a=fingerprint:sha-256 00:3E:DC:02:92:60:97:0D:D8:F7:7F:E9:AD:41:46:CD:B5:FC:33:35:3F:5C:C4:BC:CD:85:17:96:F8:D6:14:57
- a=group:BUNDLE sdparta_0
- a=ice-options:trickle
- a=msid-semantic:WMS *
- m=video 40601 UDP/TLS/RTP/SAVPF 120 121 126 97
- c=IN IP4 192.168.1.230
- a=sendrecv
- a=end-of-candidates
- a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
- a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
- a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
- a=extmap:6/sendonly urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
- a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
- a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
- a=fmtp:120 max-fs=12288;max-fr=60
- a=fmtp:121 max-fs=12288;max-fr=60
- a=ice-pwd:f317cfe0aaa529381815e208a6cdec19
- a=ice-ufrag:07366c17
- a=mid:sdparta_0
- a=msid:{8de6968c-0ea9-4a19-aaa6-51355d4a5f4e} {9bdb3a96-19f3-4730-8459-e56db515b5da}
- a=rid:hi send
- a=rid:mid send
- a=rid:lo send
- a=rtcp-fb:120 nack
- a=rtcp-fb:120 nack pli
- a=rtcp-fb:120 ccm fir
- a=rtcp-fb:120 goog-remb
- a=rtcp-fb:121 nack
- a=rtcp-fb:121 nack pli
- a=rtcp-fb:121 ccm fir
- a=rtcp-fb:121 goog-remb
- a=rtcp-fb:126 nack
- a=rtcp-fb:126 nack pli
- a=rtcp-fb:126 ccm fir
- a=rtcp-fb:126 goog-remb
- a=rtcp-fb:97 nack
- a=rtcp-fb:97 nack pli
- a=rtcp-fb:97 ccm fir
- a=rtcp-fb:97 goog-remb
- a=rtcp-mux
- a=rtpmap:120 VP8/90000
- a=rtpmap:121 VP9/90000
- a=rtpmap:126 H264/90000
- a=rtpmap:97 H264/90000
- a=setup:actpass
- a=simulcast: send rid=hi;mid;lo
- a=ssrc:252082699 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}
- a=ssrc:2611961929 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}
- a=ssrc:1452646660 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}
- In Firefox, the important simulcast bits are the a=rid  lines and the a=simulcast  line (4th from the bottom). Note: this is from an older version of the specification and subject to change.

webrtc hack
We need to convince our peer that it is actually receiving three different video streams  – a low, medium, and high bitrates – instead of just one. To accomplish that, we need to create our own SDP containing a different mapping from SSRC to track. This is a bit finicky but this site is called webrtchacks for a reason!
We end up transforming the Firefox offer into this:

- Firefox SDP with three separate video m-lines
- v=0
- o=mozilla...THIS_IS_SDPARTA-61.0 8324701712193024513 0 IN IP4 0.0.0.0
- s=-
- t=0 0
- a=sendrecv
- a=fingerprint:sha-256 00:3E:DC:02:92:60:97:0D:D8:F7:7F:E9:AD:41:46:CD:B5:FC:33:35:3F:5C:C4:BC:CD:85:17:96:F8:D6:14:57
- a=group:BUNDLE sdparta_0 sdparta_1 sdparta_2
- a=msid-semantic:WMS *
- m=video 9 UDP/TLS/RTP/SAVPF 120 121 126 97
- c=IN IP4 0.0.0.0
- a=sendrecv
- a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
- a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
- a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
- a=extmap:6/sendonly urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
- a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
- a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
- a=fmtp:120 max-fs=12288;max-fr=60
- a=fmtp:121 max-fs=12288;max-fr=60
- a=ice-pwd:f317cfe0aaa529381815e208a6cdec19
- a=ice-ufrag:07366c17
- a=mid:sdparta_0
- a=msid:low low
- a=rtcp-fb:120 nack
- a=rtcp-fb:120 nack pli
- a=rtcp-fb:120 ccm fir
- a=rtcp-fb:120 goog-remb
- a=rtcp-fb:121 nack
- a=rtcp-fb:121 nack pli
- a=rtcp-fb:121 ccm fir
- a=rtcp-fb:121 goog-remb
- a=rtcp-fb:126 nack
- a=rtcp-fb:126 nack pli
- a=rtcp-fb:126 ccm fir
- a=rtcp-fb:126 goog-remb
- a=rtcp-fb:97 nack
- a=rtcp-fb:97 nack pli
- a=rtcp-fb:97 ccm fir
- a=rtcp-fb:97 goog-remb
- a=rtcp-mux
- a=rtpmap:120 VP8/90000
- a=rtpmap:121 VP9/90000
- a=rtpmap:126 H264/90000
- a=rtpmap:97 H264/90000
- a=setup:actpass
- a=ssrc:252082699 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}
- m=video 9 UDP/TLS/RTP/SAVPF 120 121 126 97
- c=IN IP4 0.0.0.0
- a=sendrecv
- a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
- a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
- a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
- a=extmap:6/sendonly urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
- a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
- a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
- a=fmtp:120 max-fs=12288;max-fr=60
- a=fmtp:121 max-fs=12288;max-fr=60
- a=ice-pwd:f317cfe0aaa529381815e208a6cdec19
- a=ice-ufrag:07366c17
- a=mid:sdparta_1
- a=msid:mid mid
- a=rtcp-fb:120 nack
- a=rtcp-fb:120 nack pli
- a=rtcp-fb:120 ccm fir
- a=rtcp-fb:120 goog-remb
- a=rtcp-fb:121 nack
- a=rtcp-fb:121 nack pli
- a=rtcp-fb:121 ccm fir
- a=rtcp-fb:121 goog-remb
- a=rtcp-fb:126 nack
- a=rtcp-fb:126 nack pli
- a=rtcp-fb:126 ccm fir
- a=rtcp-fb:126 goog-remb
- a=rtcp-fb:97 nack
- a=rtcp-fb:97 nack pli
- a=rtcp-fb:97 ccm fir
- a=rtcp-fb:97 goog-remb
- a=rtcp-mux
- a=rtpmap:120 VP8/90000
- a=rtpmap:121 VP9/90000
- a=rtpmap:126 H264/90000
- a=rtpmap:97 H264/90000
- a=setup:actpass
- a=ssrc:2611961929 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}
- m=video 9 UDP/TLS/RTP/SAVPF 120 121 126 97
- c=IN IP4 0.0.0.0
- a=sendrecv
- a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
- a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
- a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
- a=extmap:6/sendonly urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
- a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
- a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
- a=fmtp:120 max-fs=12288;max-fr=60
- a=fmtp:121 max-fs=12288;max-fr=60
- a=ice-pwd:f317cfe0aaa529381815e208a6cdec19
- a=ice-ufrag:07366c17
- a=mid:sdparta_2
- a=msid:hi hi
- a=rtcp-fb:120 nack
- a=rtcp-fb:120 nack pli
- a=rtcp-fb:120 ccm fir
- a=rtcp-fb:120 goog-remb
- a=rtcp-fb:121 nack
- a=rtcp-fb:121 nack pli
- a=rtcp-fb:121 ccm fir
- a=rtcp-fb:121 goog-remb
- a=rtcp-fb:126 nack
- a=rtcp-fb:126 nack pli
- a=rtcp-fb:126 ccm fir
- a=rtcp-fb:126 goog-remb
- a=rtcp-fb:97 nack
- a=rtcp-fb:97 nack pli
- a=rtcp-fb:97 ccm fir
- a=rtcp-fb:97 goog-remb
- a=rtcp-mux
- a=rtpmap:120 VP8/90000
- a=rtpmap:121 VP9/90000
- a=rtpmap:126 H264/90000
- a=rtpmap:97 H264/90000 
- a=setup:actpass
- a=ssrc:1452646660 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}


- v=0
- o=mozilla...THIS_IS_SDPARTA-61.0 8324701712193024513 0 IN IP4 0.0.0.0
- s=-
- t=0 0
- a=sendrecv
- a=fingerprint:sha-256 00:3E:DC:02:92:60:97:0D:D8:F7:7F:E9:AD:41:46:CD:B5:FC:33:35:3F:5C:C4:BC:CD:85:17:96:F8:D6:14:57
- a=group:BUNDLE sdparta_0 sdparta_1 sdparta_2
- a=msid-semantic:WMS *
- m=video 9 UDP/TLS/RTP/SAVPF 120 121 126 97
- c=IN IP4 0.0.0.0
- a=sendrecv
- a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
- a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
- a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
- a=extmap:6/sendonly urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
- a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
- a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
- a=fmtp:120 max-fs=12288;max-fr=60
- a=fmtp:121 max-fs=12288;max-fr=60
- a=ice-pwd:f317cfe0aaa529381815e208a6cdec19
- a=ice-ufrag:07366c17
- a=mid:sdparta_0
- a=msid:low low
- a=rtcp-fb:120 nack
- a=rtcp-fb:120 nack pli
- a=rtcp-fb:120 ccm fir
- a=rtcp-fb:120 goog-remb
- a=rtcp-fb:121 nack
- a=rtcp-fb:121 nack pli
- a=rtcp-fb:121 ccm fir
- a=rtcp-fb:121 goog-remb
- a=rtcp-fb:126 nack
- a=rtcp-fb:126 nack pli
- a=rtcp-fb:126 ccm fir
- a=rtcp-fb:126 goog-remb
- a=rtcp-fb:97 nack
- a=rtcp-fb:97 nack pli
- a=rtcp-fb:97 ccm fir
- a=rtcp-fb:97 goog-remb
- a=rtcp-mux
- a=rtpmap:120 VP8/90000
- a=rtpmap:121 VP9/90000
- a=rtpmap:126 H264/90000
- a=rtpmap:97 H264/90000
- a=setup:actpass
- a=ssrc:252082699 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}
- m=video 9 UDP/TLS/RTP/SAVPF 120 121 126 97
- c=IN IP4 0.0.0.0
- a=sendrecv
- a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
- a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
- a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
- a=extmap:6/sendonly urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
- a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
- a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
- a=fmtp:120 max-fs=12288;max-fr=60
- a=fmtp:121 max-fs=12288;max-fr=60
- a=ice-pwd:f317cfe0aaa529381815e208a6cdec19
- a=ice-ufrag:07366c17
- a=mid:sdparta_1
- a=msid:mid mid
- a=rtcp-fb:120 nack
- a=rtcp-fb:120 nack pli 
- a=rtcp-fb:120 ccm fir
- a=rtcp-fb:120 goog-remb
- a=rtcp-fb:121 nack
- a=rtcp-fb:121 nack pli
- a=rtcp-fb:121 ccm fir
- a=rtcp-fb:121 goog-remb
- a=rtcp-fb:126 nack
- a=rtcp-fb:126 nack pli
- a=rtcp-fb:126 ccm fir
- a=rtcp-fb:126 goog-remb
- a=rtcp-fb:97 nack
- a=rtcp-fb:97 nack pli
- a=rtcp-fb:97 ccm fir
- a=rtcp-fb:97 goog-remb
- a=rtcp-mux
- a=rtpmap:120 VP8/90000
- a=rtpmap:121 VP9/90000
- a=rtpmap:126 H264/90000
- a=rtpmap:97 H264/90000
- a=setup:actpass
- a=ssrc:2611961929 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}
- m=video 9 UDP/TLS/RTP/SAVPF 120 121 126 97
- c=IN IP4 0.0.0.0
- a=sendrecv
- a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
- a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
- a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
- a=extmap:6/sendonly urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
- a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
- a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
- a=fmtp:120 max-fs=12288;max-fr=60
- a=fmtp:121 max-fs=12288;max-fr=60
- a=ice-pwd:f317cfe0aaa529381815e208a6cdec19
- a=ice-ufrag:07366c17
- a=mid:sdparta_2
- a=msid:hi hi
- a=rtcp-fb:120 nack
- a=rtcp-fb:120 nack pli
- a=rtcp-fb:120 ccm fir
- a=rtcp-fb:120 goog-remb
- a=rtcp-fb:121 nack
- a=rtcp-fb:121 nack pli
- a=rtcp-fb:121 ccm fir
- a=rtcp-fb:121 goog-remb
- a=rtcp-fb:126 nack
- a=rtcp-fb:126 nack pli
- a=rtcp-fb:126 ccm fir
- a=rtcp-fb:126 goog-remb
- a=rtcp-fb:97 nack
- a=rtcp-fb:97 nack pli
- a=rtcp-fb:97 ccm fir
- a=rtcp-fb:97 goog-remb
- a=rtcp-mux
- a=rtpmap:120 VP8/90000
- a=rtpmap:121 VP9/90000
- a=rtpmap:126 H264/90000
- a=rtpmap:97 H264/90000
- a=setup:actpass
- a=ssrc:1452646660 cname:{004db4a6-943d-496b-9e8d-1689f7be7355}
- This shows three different media sections (as specified in unified plan) which will trigger the ‘track’ event three times at the receiver, giving us three different MediaStream  objects to attach to video elements and will also allow making graphs for the individual bitrates using the getStats  API.

- Refer to the source code for the SDP munging for how to create this.

- Tuning the bitrate for individual layers
- Chrome has long used a hardcoded table for the simulcast bitrates of the individual spatial layers.

- C++
- // These tables describe from which resolution we can use how many
- // simulcast layers at what bitrates (maximum, target, and minimum).
- // Important!! Keep this table from high resolution to low resolution.
- // clang-format off
- const SimulcastFormat kSimulcastFormats[] = {
-   {1920, 1080, 3, 5000, 4000, 800},
-   {1280, 720, 3,  2500, 2500, 600},
-   {960, 540, 3, 900, 900, 450},
-   {640, 360, 2, 700, 500, 150},
-   {480, 270, 2, 450, 350, 150},
-   {320, 180, 1, 200, 150, 30},
-   {0, 0, 1, 200, 150, 30}
- };

- // These tables describe from which resolution we can use how many
- // simulcast layers at what bitrates (maximum, target, and minimum).
- // Important!! Keep this table from high resolution to low resolution.
- // clang-format off
- const SimulcastFormat kSimulcastFormats[] = {
-   {1920, 1080, 3, 5000, 4000, 800},
-   {1280, 720, 3,  2500, 2500, 600},
-   {960, 540, 3, 900, 900, 450},
-   {640, 360, 2, 700, 500, 150},
-   {480, 270, 2, 450, 350, 150},
-   {320, 180, 1, 200, 150, 30},
-   {0, 0, 1, 200, 150, 30}
- };
The table shows the resolution (e.g. 1920×1080), the number of spatial layers (3) as well as the maximum, optimal and minimum bitrate.

Thanks to setParameters  we can now deviate from the bitrates defined in this table and get creative. If you run the example without any modifications, you can see that Simulcast sends at a bitrate of about 3.2mbps. This is split up into three different spatial streams of around 150kbps, 500kbps and 2500kbps which matches the settings in the table.

Let’s modify this by pasting this JavaScript into the console:

- JavaScript
- var p = pc1.getSenders()[0].getParameters();
- p.encodings[1].maxBitrate = 300*1000;
- p.encodings[2].maxBitrate = 400*1000;
- pc1.getSenders()[0].setParameters(p)
-   .catch(e => console.error(e))

- var p = pc1.getSenders()[0].getParameters();
- p.encodings[1].maxBitrate = 300*1000;
- p.encodings[2].maxBitrate = 400*1000;
- pc1.getSenders()[0].setParameters(p)
-   .catch(e => console.error(e))
This sets the target bitrate of the mid-resolution (640×360) spatial layer to 300kbps and the target bitrate of the 720p stream to 400kbps. These bitrates are matched quite well by the encoder and their quality is surprisingly good, even at 400kbps for the 720p stream.

If we reduce the target bitrate for the 720p layer even more to 200kbps we can see a visual degradation as the framerate drops because only the base temporal layer is sent. 200kbps is about as low as you can go with a 720p stream…
