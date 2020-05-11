**simulcast**

Most of you are probably familiar with the typical SFU-style user interface that was first popularized in the consumer market with Google Hangouts and is used by Jitsi Meet and other Services. Front and center with the vast majority of screen real estate is the video of whoever is the current active speaker.  All the other participants are seen in their own thumbnail, usually on the right or across the bottom. We want the active speaker’s video in the middle to look great so that is high resolution. The thumbnails on the bottom/right are small, so high resolution there would be a waste of bandwidth.  To optimize for these different modes we need each sender’s video in multiple resolutions. Thankfully this is already a solved problem with simulcast!

With simulcast, all senders encode 3 different resolutions and send them to the SFU.  The SFU decides which streams to forward to each receiver. If a participant is the active speaker, we try and forward the highest quality stream they’re sending to others to see on their main stage. If a participant is going to be seen in the thumbnails on the right, then we forward their lowest quality stream.


**mulcast Trade-offs**
Simulcast is a great mechanism for optimizing download bandwidth.  However, like most things in life, simulcast involves trade offs – encoding 3 streams is more CPU-intensive than encoding a single stream. You can see this in the chrome://webrtc-internals stats below where the CPU usage is a few percentage points higher using simulcast:

	
CPU usage without simulcast	CPU usage with simulcast
It also involves sending more bits:

*Send bitrate without simulcast (~2,5M bits/sec)	Send bitrate with simulcast ~ (3M bits/sec)*

**Stream Suspension**
So does this mean simulcast is less efficient for the user? On the contrary, since we can control the simulcast streams individually, simulcast gives us the opportunity to actually save both CPU and bits by turning off the layers aren’t in use. If you’re not the active speaker, then 2 of your 3 layers aren’t needed at all!  

**mplementing Suspension**
Now let’s see if we can integrate this into the actual code.  There are 2 problems to solve here:

-a) On the SFU – figure out when streams aren’t being used and let clients know
-b) Refresh the stream, delete track recreate with same id 
-c) On the client – shut off streams when they aren’t being used and start them back up when they’re needed again


-a) The first problem was straightforward to solve – the clients already explicitly request high quality streams from participants when they become the active speaker so we can tell senders when their high quality streams are being used and when they are not via data channel messages.

The Client
First Attempt
We had an idea for the second problem as well. We know Chrome will suspend transmission of simulcast streams when the available bandwidth drops, so what if we just capped the available bandwidth?  We can do this by putting a bandwidth limit in the remote SDP:


Capping the maximum send bandwidth using SDP

The b=AS  line will cap the available bandwidth to 200kbps.

Let’s give that a try and see how it looks:

	
CPU usage after SDP bandwidth restriction	Send bitrate after SDP bandwidth restriction
Great!  This is exactly what we were hoping for: it matches the test results we had from earlier!  Now let’s remove the cap to simulate when someone becomes the active speaker and we want their high quality streams:

		
CPU usage when the cap is removed	Send bitrate when the cap is removed	Send frame height when the cap is removed
Great, it comes back!  But there’s a problem…it takes over 30 seconds to get back to high quality.  This means when someone becomes the active speaker, their video quality on the main stage will be poor for at least 30 seconds.  This won’t work. Why is it so slow?

If you’ve ever done network impairment testing with Chrome, you know that it applies lots of logic to prevent oversending. It is very cautious to ramp up the send bitrate for fear of losing packets.  What we’ve basically done via our SDP parameter is make Chrome think the network’s capacity for packets is very low (200 kbps) so when we remove it, Chrome cautiously increases the bitrate while it figures out how much it can actually send.  This makes a lot of sense when the network is having issues, but for our use case it’s a dealbreaker.



-c)
The WebRTC team recently put out a PSA about RTCRtpSender. Support for modifying encoding parameters landing in Chrome 69.  This has an API that gives us control over individual simulcast encodings, including whether or not they are enabled!  So, when we find out we’re not going to be on the main stage, the client can do:

const videoSender = peerconnection.getSenders().find(sender => sender.track.kind === 'video');
const parameters = videoSender.getParameters();
parameters.encodings[1].active = false;
parameters.encodings[2].active = false;
videoSender.setParameters(parameters);

const videoSender = peerconnection.getSenders().find(sender => sender.track.kind === 'video');
const parameters = videoSender.getParameters();
parameters.encodings[1].active = false;
parameters.encodings[2].active = false;
videoSender.setParameters(parameters);



Simulcast is a way to send multiple streams at the same time (typically different qualities of the same source), thus allowing recipients, with the help of components like SFUs, to receive one or another depending on their bandwidth or application requirements. For instance, a user may decide to receive the low quality of a conference participant because they don’t have enough bandwidth to receive the good quality one; or maybe they want the low quality version because they’re currently displayed in a thumbnail, and anything with a higher resolution would be a waste of data. Bottom line, what’s really important is that simulcast gives you the option to provide different alternatives that recipients can then take advantage of: as such, it’s a common feature in most conferencing applications.

When it comes to how simulcast is implemented in browsers there are three different flavours. Well, it’s actually two, but one of them is implemented in a different enough way by Chrome and Firefox to basically justify treating it as two different approaches. To summarize, we have:

**good ol’ fashioned SDP munging (Chrome, Safari);**
**older version of rid-based simulcasting (Firefox);**
**spec version of rid-based simulcasting (Chrome >= 74).**

We’ve supported the first two approaches in Mediaserver for a while already, but we now (finally!) support them all, so let’s have a look at what those approaches are, what we had to do in order to get them to work, and what may happen in the future.

SDP munging
SDP munging is how “traditionally” simulcast was first introduced in WebRTC. At the time, simulcast was a hidden and not advertised feature, only meant to be used by Hangouts: as such, the only way to enable it outside of the context of a Hangouts session was to manually modify the SDP in a similar way, in order to add the required fields before passing the SDP back to the browser. This is what the “SDP munging” term refers to: you create your offer, tweak it, and then enforce it via a setLocalDescription.

In order to enable simulcasting that way, what is needed is basically adding additional SSRCs for the extra streams, and put them in a SIM SSRC group. Let’s say we got this SDP from Chrome (just the relevant snippet for brevity):

- a=ssrc:659652645 cname:Taj3/ieCnLbsUFoH
- a=ssrc:659652645 msid:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk 028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:659652645 mslabel:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk
- a=ssrc:659652645 label:028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:98148385 cname:Taj3/ieCnLbsUFoH
- a=ssrc:98148385 msid:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk 028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:98148385 mslabel:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk
- a=ssrc:98148385 label:028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc-group:FID 659652645 98148385

What this tells us is that video will use SSRC 659652645, and the FID ssrc-group explains that SSRC 98148385 will be used for retransmissions when needed. As such, there’s only a single video stream for now: no trace of simulcast yet. In order to do that, we have to manipulate the SDP to add those streams ourselves. Specifically, we’ll need to add two more SSRCs (and their related retransmission SSRCs), and add a new SIM ssrc-group to tie them together:

a- =ssrc:659652645 cname:Taj3/ieCnLbsUFoH
- a=ssrc:659652645 msid:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk 028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:659652645 mslabel:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk
- a=ssrc:659652645 label:028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:98148385 cname:Taj3/ieCnLbsUFoH
- a=ssrc:98148385 msid:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk 028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:98148385 mslabel:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk
- a=ssrc:98148385 label:028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:1982135572 cname:Taj3/ieCnLbsUFoH
- a=ssrc:1982135572 msid:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk 028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:1982135572 mslabel:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk
- a=ssrc:1982135572 label:028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:2523084908 cname:Taj3/ieCnLbsUFoH
- a=ssrc:2523084908 msid:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk 028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:2523084908 mslabel:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk
- a=ssrc:2523084908 label:028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:3604909222 cname:Taj3/ieCnLbsUFoH
- a=ssrc:3604909222 msid:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk 028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:3604909222 mslabel:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk
- a=ssrc:3604909222 label:028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:1893605472 cname:Taj3/ieCnLbsUFoH
- a=ssrc:1893605472 msid:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk 028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc:1893605472 mslabel:i1zOaprU7rZzMDaOXFdqwkq7Q6wP6f3cgUgk
- a=ssrc:1893605472 label:028ab73b-cdd0-4b61-a282-ea0ed0c6a9bb
- a=ssrc-group:SIM 659652645 1982135572 3604909222
- a=ssrc-group:FID 659652645 98148385
- a=ssrc-group:FID 1982135572 2523084908
- a=ssrc-group:FID 3604909222 1893605472


That’s it! When you do that, the browser enables simulcasting on the stream, meaning that it will start encoding three different streams at different quality and bitrate. Temporal layers are enabled too for each of the substreams, when VP8 is used, which is a nice added value.



Simulcasting in EchoTest demo (SDP munging)
As you can see (and as you probably knew already, if you ever played with the simulcast support in Mediaserver), additional controls appear in the demo when you enable simulcast. The buttons allow you to select which substream or temporal layer you want to receive, and give you some feedback on what you’re getting right now: in the snapshot above, we’re receiving the high quality substream (SL2) and all temporal layers (TL2); clicking the other buttons would allow you to receive a different one (e.g., SL1 for the medium quality substream), assuming the browser is sending them.

In Mediaserver, this is made possible by the fact that those additional SSRCs are properly detected, which in turn enable the simulcast support. Plugins are notified about this accordingly, and SSRCs are used to automatically demultiplex incoming packets, i.e., to figure out to which substreams they refer to. Specifically, Chrome and Safari list SSRCs in increasing order (low, medium and high quality), which makes it easy for Mediaserver and its plugins to make assumptions on how to demultiplex the traffic. Temporal layers, instead, are detected by looking at the payload itself: at the time of writing, temporal layers are only enabled for VP8 when simulcasting, which means that H.264 does not add them instead.

Enter rid
While SDP munging “works”, it’s definitely not the proper way to do simulcasting. Without delving in too many details, the specification explains that you should configure the different encodings programmatically via API instead: this will in turn result in those encodings to be advertised in the SDP as different rid (restriction identifiers), whose values will also appear in the related RTP packets as RTP extensions.

The first browser to implement support for this simulcast approach, about 3 years ago, was Firefox in version 46. The way simulcast is enabled in Firefox right now isn’t entirely compliant to the spec (more on that later), which is why I started by saying there are three ways of doing simulcast right now instead of just two, but it’s really close. One of the differences is in how you use the API to configure the different encodings, which in Firefox is done after the track has been added and before the SDP is generated, e.g.:

var sender = pc.getSenders().find(s => s.track.kind == "video");
var parameters = sender.getParameters();
if(!parameters)
	parameters = {};
parameters.encodings = [
	{ rid: "h", active: true, maxBitrate: 900000 },
	{ rid: "m", active: true, maxBitrate: 300000, scaleResolutionDownBy: 2 },
	{ rid: "l", active: true, maxBitrate: 100000, scaleResolutionDownBy: 4 }
];
sender.setParameters(parameters);
What the above snippet does is finding the video track we want to enable simulcast for, retrieving its parameters, replacing the encodings with our own configuration and then updating the parameters. In this case, we’re creating three different substreams:

a high quality stream (rid=h, highest resolution, maximum bitrate of 900kbps);
a medium quality stream (rid=m, 1/2 resolution, maximum bitrate of 300kbps);
a low quality stream (rid=l, 1/4 resolution, maximum bitrate of 100kbps).


Whatever the encodings, once simulcast is enabled part of the SDP offer in Firefox will look like this:

- a=extmap:6/sendonly urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
- [..]
- a=rid:h send
- a=rid:m send
- a=rid:l send
- a=simulcast: send rid=h;m;l
- a=ssrc:2232294953 cname:{d31e1342-72d7-45e3-9b8d-9419cf7121b6}
- a=ssrc:2149721810 cname:{d31e1342-72d7-45e3-9b8d-9419cf7121b6}
- a=ssrc:2539596807 cname:{d31e1342-72d7-45e3-9b8d-9419cf7121b6}

As you can see, while Firefox is still giving us the three different SSRCs that will be used for the three substreams, there is no SIM ssrc-group: the rid values are used instead for the purpose, specifically in dedicated a=rid attributes to name them, and in a a=simulcast line to tie them together. Notice that this syntax, while very close to the current spec, is not 100% compliant: in fact, at the time of writing Firefox still only supports version -03 of the simulcast draft, which is apparent from the extra rid= in the a=simulcast line that shouldn’t be there. Finally, since those rid values will need to be included in RTP packets, the related RTP extension (urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id) is negotiated as well.



Simulcasting in EchoTest demo (rid in Firefox)
From a visual perspective, nothing changes, and that’s on purpose: the functionality, as far as end users are concerned, is exactly the same, and so are the buttons and feedback related to the simulcast functionality.

Behind the curtains, though, things are different. First of all, the answer Mediaserver will send back will be in line with what Firefox sent, i.e.:

- a=extmap:6/recvonly urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
- [..]
- a=rid:h recv
- a=rid:m recv
- a=rid:l recv
- a=simulcast: recv rid=h;m;l


Very simply, Mediaserver accepts the RTP extension and the simulcast offer by explaining it will handle those streams in receive mode.

Once that happens, Mediaserver inspects all incoming RTP packets to look for the rid in RTP extensions: when one is found, it is compared to the negotiated values. This is needed to figure out which substream the specific packet belongs to, and which SSRC is being used for the related RTP session: in fact, while Firefox advertises those SSRCs in the SDP, there is no explicit relationship between them and the rids, and just relying on their order may lead to wrong assumptions. Once these relationships are established properly, Mediaserver can start using SSRCs for demultiplexing packets instead (which is more efficient than comparing strings), at least until an unknown SSRC is detected which would involve looking for the right rid value in RTP extensions again.

A world without SSRCs!
As anticipated, while Firefox implements rid-based simulcasting almost the right way, it’s not currently in line with the spec. Recently, more precisely from M74, Chrome started implementing the spec version of simulcast instead, which replaced the legacy SDP munging it has been doing so far.

In general, the same exact concepts we introduced in the previous section (API to configure encodings, rid values, and so on) are still valid. There are some key differences, though. The first and important one is that, per the spec, the so-called simulcast envelope can only be created when adding the new video track via transceivers: this is a key difference from the previous section, where we saw how Firefox would first add the track, and only after that manipulate its encoding properties. Looking at the code, and preserving the same encoding configuration properties for a better understanding of the differences, it looks like this:

pc.addTransceiver(track, {
	direction: "sendrecv",
	streams: [stream],
	sendEncodings: [
		{ rid: "h", active: true, maxBitrate: 900000 },
		{ rid: "m", active: true, maxBitrate: 300000, scaleResolutionDownBy: 2 },
		{ rid: "l", active: true, maxBitrate: 100000, scaleResolutionDownBy: 4 }
	]
});
As you can see, rather than looking for an existing sender and updating its parameters, in this snippet we’re providing the desired encodings right away when adding the new transceiver.

When this is done, this is what Chrome would generate in the SDP:

- a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
- a=extmap:5 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id
- [..]
- a=rid:h send
- a=rid:m send
- a=rid:l send
- a=simulcast:send h;m;l


Notice anything different? Well, for a start there’s another rid-related RTP extension (urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id), but that’s only relevant when doing retransmissions via rtx (which Firefox currently doesn’t support), so it’s only of relative interest here. Then we can see how Chrome doesn’t put the unneeded rid= in the a=simulcast line, as it implements the latest version of the simulcast draft. But there’s something else, more disconcerting…

**WHAT, NO SSRC?!**
As you can guess, this took more than a few (yours truly included) by surprise. Most, if not all, of the WebRTC server side implementations typically rely on SSRCs to work, as they’re needed for telling one packet from another and know what they’re about. As such, this was seen as a possibly breaking change by many, which also led to a lengthy discussion on the W3C WebRTC github repo and further exchanges during a W3C WebRTC WG intering meeting.

As we’ve seen from the previous section, though, you don’t really need SSRCs in the SDP to be able to demultiplex traffic if rid values are advertised and used: in that case, in fact, inspecting RTP extensions to derive the proper rid/SSRC mappings implicitly can help applications still work as expected. This was exactly what I focused on a couple of weeks ago, which led me to publish a pull request with a patch for Mediaserver to properly implement this functionality, which we already merged.



Simulcasting in EchoTest demo (rid in Chrome)
The outcome is, again, visually exactly the same as the previous two tests, which is what we expected: the user experience remains the same, with the ability of selecting what to receive back.

In Mediaserver, though, new pieces in the code allow for all this to work. First of all, Mediaserver answers as it’s supposed to do:

a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
a=extmap:5 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id
[..]
- a=rid:h recv
- a=rid:m recv
- a=rid:l recv
- a=simulcast: recv h;m;l
by negotiating support for the relevant extensions and acknowledging the ability to receive the simulcast streams by their rid values. Then, once media starts flowing, the same things we introduced in the previous section start happening: we find out about SSRCs by looking at the rid values in RTP extensions, and then use that info to demultiplex traffic. It’s worthwhile to point out that, since Chrome implements retransmissions via rtx, the rid also helps identifying the SSRCs used for retransmissions, which is again very important as those values are not advertised anywhere in the SDP either.

There are some more changes that happened within Mediaserver as part of that effort (more specifically, in terms of how the Mediaserver core and the plugins exchange information about simulcast support), but that’s not relevant to this article. You can refer to the previously referenced pull request for more info about that.

**What’s next?**
While simulcasting works nicely most of the times, it’s not without issues.

That said, one of the key issues in simulcasting today is how to configure, or figure out, the ordering of streams: e.g., which one will be low quality, which one will be high quality, and so on. As we’ve seen in the previous sections, when SDP munging it’s implicit that the first SSRC you add will be for the low quality feed, and the last for the high quality one, which means the order in the session description is important. That said, in Firefox and Chrome >= 74 this assumption is not always true, and can lead to confusion. The confusion becomes even worse when you think about which streams should be preferred over others when bandwidth is insufficient for all of them: should the browser always send a low quality feed, to have a fallback, or should it try to always send other layers instead, no matter the ordering of streams in terms of resolution/bitrate? 

Most importantly, as anticipated the lack of SSRCs in the new rid-based simulcast support in Chrome was not uneventful. There has been a lot of debate on whether they should be in the SDP anyway or not, and what would happen if they just disappeared. The specification is clear in that regard: SSRCs in SDP are not really needed, as mid/rid can help demultiplex traffic and associate it both to the right m-line, and to the right substream when available. On the other end, though, as anticipated almost all existing WebRTC implementations rely heavily on SSRCs to work, and a big change like that can’t happen overnight: we ourselves had to tweak the Mediaserver code to make it work with SSRC-less simulcasting, and that won’t be enough if SSRCs will disappear entirely, e.g., in non-simulcast scenarios as well. This becomes worrisome when you realise that “SSRC-less” only works if mid/rid extensions are negotiated, and that if they’re not the session will be hopelessly broken since there wouldn’t be any mechanism at all for demultiplexing traffic: considering WebRTC endpoints may be communicating with, and receiving media from, legacy endpoints that don’t really know what mid or rid are, this could be very problematic. As such, a transition phase will probably be needed to ensure a smooth transition for most, and avoid breaking a huge chunk of applications that do work nicely at the moment.

**What’s next in Mediaserver, then?**
Well, we’ve already given this some thought, and discussed this with other experts in the community. As anticipated, we don’t really have an issue with incoming media, as that’s been addressed during the related simulcast effort. Issues may arise for outgoing traffic, though, for a few reasons that would be boring to list here and now.

The only reasonable solution to that and to the SSRC dilemma will probably be implementing proper mid/rid extension support for outgoing traffic as well (as we currently only do that for incoming RTP packets, but disable them on outgoing offers), which will hopefully ensure that things won’t break just because we’re routing the wrong RTP extensions due to the flexible logic we have in plugins. This would be especially needed for all those scenarios that involve media coming from non-WebRTC sources, e.g., SIP gatewaying or broadcasting via the Streaming plugin.

As such, expect some effort in that direction soon!





