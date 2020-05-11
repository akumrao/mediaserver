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

-On the SFU – figure out when streams aren’t being used and let clients know
-On the client – shut off streams when they aren’t being used and start them back up when they’re needed again

The SFU
The first problem was straightforward to solve – the clients already explicitly request high quality streams from participants when they become the active speaker so we can tell senders when their high quality streams are being used and when they are not via data channel messages.

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
