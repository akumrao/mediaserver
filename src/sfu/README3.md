Mapping Address(yes or no) Maping Port(yes or no)

Filtering Address(yes or no) Filtering Port(yes or no)

2^4=16

So, there are 16 NAT types defined in RFC 5780


TURN
Some routers using NAT employ a restriction called ‘Symmetric NAT ( filter by port and addresss)’. This means the router will only accept connections from peers you’ve previously connected to.

Traversal Using Relays around NAT (TURN) is meant to bypass the Symmetric NAT restriction by opening a connection with a TURN server and relaying all information through that server. You would create a connection with a TURN server and tell all peers to send packets to the server which will then be forwarded to you. This obviously comes with some overhead so it is only used if there are no other alternatives.




https://www.netmanias.com/en/?m=view&id=techdocs&no=6065&xtag=nat-network-protocol&xref=stun-rfc-3489-vs-stun-rfc-5389-5780

What are the differences between NAT types defined in RFC 3489 and RFC 5780?
To understand the differences to be explained below, you must be familiar with the Mapping Behavior and Filtering Behavior of a NAT that we covered last time.

 

What is STUN?
 

STUN is a protocol that allows two devices (P2P devices) to discover the presence and types of a NAT between them, and to find out what External IP address and Port are to be replaced by the NAT, for P2P communication between the two devices.
 
STUN was first defined in RFC 3489 (standards) back in 2003, and then revised two times once in RFC 5389 (standards) in 2008 and again in RFC 5780 (experimental) in 2010.  
According to RFC 5389, "classic STUN's  algorithm for classification of NAT types (defined in RFC 3489) was found to be faulty, as many NATs (available in the market) did not fit cleanly into the (four) types defined there."
To resolve this problem, classic STUN was modified in RFC 5389 (i.e. to add attributes to Binding messages). And in RFC 5780, NAT types were re-defined and the algorithm (method) for classification of NAT types was modified. 
 
Both RFC 3489 and 5389 use the same term "STUN", but in two different meanings as follows:

RFC 3489: STUN - Simple Traversal of User Datagram Protocol (UDP) Through Network Address Translators (NATs)
RFC 5389: Session Traversal Utilities for NAT (STUN) 
 

 

NAT Types Defined in RFC 3489 and 5780

 

1. Full Cone

 
[Mapping Behavior] A full cone NAT is one where all requests from the same internal IP address and port are mapped to the same external IP address and port.  [Filtering Behavior] Furthermore, any external host can send a packet to the internal host, by sending a packet to the mapped external address. (RFC 3489)

 

A full cone NAT in RFC 3489 corresponds to a NAT that uses Endpoint-Independent Mapping ("EIM") and Endpoint-Independent Filtering ("EIF") in RFC 5780. 

Mapping Behavior: The NAT translates any outbound packets with the same (1) source IP and (2) source port into the same Port Mapping value (e.g. translated Port = 1000 in the figure below), regardless of the destination IP or destination port of the packet.
Filtering Behavior: The NAT checks only the (1) destination IP and (2) destination port of an inbound packet to decide whether to pass the packet or not. Thus, it doesn't care about the source IP or source port values of the External Endpoint. 




https://www.3cx.com/blog/voip-howto/stun-details/

The STUN Protocol
STUN is a client-server protocol. A STUN client (typically embedded in VoIP software, such as an IP PBX or IP Phone) sends a request to a STUN server to discover its public IP and port(s), and the STUN server returns a response. There are two types of requests; Binding Requests which are typically sent over UDP, and Shared Secret Requests, which are sent over TLS (secure communication) over TCP. Shared Secret Requests ask the server to return a temporary set of credentials which are then used in a Binding Request and Binding Response exchange, for the purposes of authentication and message integrity.

Binding requests sent from the STUN client to the STUN server are used to determine the IP and port(s) bindings allocated by NAT’s. The STUN client sends a Binding Request to the STUN server, over UDP; the server examines the source IP address and port of the binding request, and copies them into a binding response that is sent back to the client. There are also Attributes (explained in more detail later in this article) in the request that allow the client to ask that the response be sent elsewhere; to a different IP address and port(s).

STUN Messages
STUN messages are TLV (type-length-value) encoded using big endian (network ordered) binary. All STUN messages start with a STUN header, followed by a STUN payload. The payload is a series of STUN attributes (explained in more detail later in this article), the set of which depends on the message type. The STUN header contains:

stun message type which typically is one of the below:
0x0001 : Binding Request
0x0101 : Binding Response
0x0111 : Binding Error Response
0x0002 : Shared Secret Request
0x0102 : Shared Secret Response
0x0112 : Shared Secret Error Response
Message length – Indicates the total length of the STUN payload in bytes but does not include the 20 bytes header.
Transaction id –Is used to correlate requests and responses.
STUN Protocol Attributes present in STUN requests and responses
Both STUN server requests and responses contain message attributes. As we will see below, some of the attributes are not mandatory, some can be found in both binding requests and binding responses, some of them can be present only in binding requests only and some other can be present in binding responses only. Below is a list of STUN attributes and a short explanation of each:

0x0001: MAPPED-ADDRESS – This attribute contains an IP address and port. It is always placed in the Binding Response, and it indicates the source IP address and port the server saw in the Binding Request sent from the client, i.e.; the STUN client’s public IP address and port where it can be reached from the internet.
0x0002: RESPONSE-ADDRESS – This attribute contains an IP address and port and is an optional attribute, typically in the Binding Request (sent from the STUN client to the STUN server). It indicates where the Binding Response (sent from the STUN server to the STUN client) is to be sent. If this attribute is not present in the Binding Request, the Binding Response is sent to the source IP address and port of the Binding Request which is attribute 0x0001: MAPPED-ADDRESS.
0x0003: CHANGE-REQUEST – This attribute, which is only allowed in the Binding Request and optional, contains two flags; to control the IP address and port used to send the response. These flags are called “change IP” and “change Port” flags. The “change IP” and “change Port” flags are useful for determining whether the client is behind a restricted cone NAT or restricted port cone NAT. They instruct the server to send the Binding Responses from a different source IP address and port.
0x0004: SOURCE-ADDRESS – This attribute is usually present in Binding Responses; it indicates the source IP address and port where the response was sent from, i.e. the IP address of the machine the client is running on (typically an internal private IP address). It is very useful as from this attribute the STUN server can detect twice NAT configurations.
0x0005: CHANGED-ADDRESS – This attribute is usually present in Binding Responses; it informs the client of the source IP address and port that would be used if the client requested the “change IP” and “change port” behaviour.
0x0006: USERNAME – This attribute is optional and is present in a Shared Secret Response with the PASSWORD attribute. It serves as a means to identify the shared secret used in the message integrity check.
0x0007: PASSWORD – This attribute is optional and only present in Shared Secret Response along with the USERNAME attribute. The value of the PASSWORD attribute is of variable length and used as a shared secret between the STUN server and the STUN client.
0x0008: MESSAGE-INTEGRITY – This attribute must be the last attribute in a STUN message and can be present in both Binding Request and Binding Response. It contains HMAC-SHA1 of the STUN message.
0x0009: ERROR-CODE – This attribute is present in the Binding Error Response and Shared Secret Error Response only. It indicates that an error has occurred and indicates also the type of error which has occurred. It contains a numerical value in the range of 100 to 699; which is the error code and also a textual reason phrase encoded in UTF-8 describing the error code, which is meant for the client.
0x000a: UNKNOWN-ATTRIBUTES – This attribute is present in the Binding Error Response or Shared Secret Error response when the error code is 420; some attributes sent from the client in the Request are unknown and the server does not understand them.
0x000b: REFLECTED-FROM – This attribute is present only in Binding Response and its use is to provide traceability so the STUN server cannot be used as part of a denial of service attack. It contains the IP address of the source from where the request came from, i.e. the IP address of the STUN client.
Common STUN Server error codes
Like many other protocols, the STUN protocol has a list of error codes. STUN protocol error codes are similar to those of HTTP or SIP. Below is a list of most common error codes encountered when using the STUN protocol. For a complete list of STUN protocol error codes refer to the STUN RFC 3489.

Error Code 400 – Bad request; the request was malformed. Client must modify request and try sending it again.
Error Code 420 – Unknown attribute; the server did not understand an attribute in the request.
Error Code 430 – Stale credentials; the shared secret sent in the request is expired; the client should obtain a new shared secret.
Error Code 432 – Missing username; the username attribute is not present in the request.
Error Code 500 – Server error; temporary error and the client should try to send the request again.


https://www.3cx.com/blog/voip-howto/stun-voip-1/
Purpose of the STUN Protocol

The main purpose of the STUN protocol is to enable a device running behind a NAT device to discover its public IP and what type of NAT is running on the gateway it is connected to. It also enables the device connected behind a gateway to discover the port translation done by the gateway itself (NAT); i.e. which port other devices can use to connect to it from outside the network. Note that gateways and routers do not always make port translations; it depends on the type of NAT they are running and how it is configured. E.g. a full cone NAT configuration does not translate ports.

STUN can also be used to refresh NAT bindings; as a keep-alive mechanism when using Restricted Cone NAT or Port Restricted cone NAT. When passing traffic through such NAT configurations, internal address and port are mapped to a specific external address and port. But if such address translation is not used after a particular amount of time (depending on the device’s configuration), such address mapping is dropped. Therefore when the internal device tries to connect again to an external entity (which could be the same entity previously it connected to) using the same internal IP and port, the router will still assign a different address mapping, i.e. a different IP and port from the previous assigned ones.

The STUN Protocol

STUN is a server-client protocol. A STUN server usually operates on both TCP and UDP and listens on port 3478. A client usually contacts the STUN server on a specific IP and port (3478) but the server can hint clients to perform tests on alternate IP address and port number too, as such port and IP are arbitrary.




**TURN**
Some routers using NAT employ a restriction called ‘Symmetric NAT’. This means the router will only accept connections from peers you’ve previously connected to.

Traversal Using Relays around NAT (TURN) is meant to bypass the Symmetric NAT restriction by opening a connection with a TURN server and relaying all information through that server. You would create a connection with a TURN server and tell all peers to send packets to the server which will then be forwarded to you. This obviously comes with some overhead so it is only used if there are no other alternatives.

First, the client contacts a TURN server with an "Allocate" request. The Allocate request asks the TURN server to allocate some of its resources for the client so that it may contact a peer. If allocation is possible, the server allocates an address for the client to use as a relay, and sends the client an "Allocation Successful" response, which contains an "allocated relayed transport address" located at the TURN server.

Second, the client sends in a CreatePermissions request to the TURN server to create a permissions check system for peer-server communications. In other words, when a peer is finally contacted and sends information back to the TURN server to be relayed to client, the TURN server uses the permissions to verify that the peer-to-TURN server communication is valid.

After permissions have been created, the client has two choices for sending the actual data, (1) it can use the Send mechanism, or (2) it can reserve a channel using the ChannelBind request. The Send mechanism is more straightforward, but contains a larger header, 36 bytes, that can substantially increase the bandwidth in a TURN relayed conversation. By contrast, the ChannelBind method is lighter: the header is only 4 bytes, but it requires a channel to be reserved which needs to be periodically refreshed, among other considerations.

Using either method, Send or channel binding, the TURN server receives the data from the client and relays it to the peer using UDP datagrams, which contain as their Source Address the "Allocated Relayed Transport Address". The peer receives the data and responds, again using a UDP datagram as the transport protocol, sending the UDP datagram to the relay address at the TURN server.

The TURN ser

The TURN protocol utilizes a TURN server to relay data from a client to any number of peers. 
A TURN client first sends a message to a TURN server to allocate an IP address and port on the TURN server that the client can use to communicate with peers. Once the allocation has succeeded, the client will use this IP address and port as its SIP URI in registrations and as its media address and port in its SDP. All data meant for the client’s peer is then encapsulated in a TURN packet and sent to the TURN server. The TURN packet also contains the destination address of the peer. The TURN server then converts this packet into a UDP packet and sends it on to the peer. In the reverse direction the TURN server receives a UDP packet from the peer and encapsulates this packet into a TURN packet and sends it to the client. The TURN packet also contains the peer’s address so that the client knows where the packet originated.

Consider using the SIP protocol where a SIP device with user Bob sits behind a NAT and wants to register its location with a SIP registrar located on the public Internet. The SIP device has a non-routable Private IP address 192.168.0.10. The SIP device registers its location with the registrar as sip:bob@192.168.0.10:5060. This tells the registrar that Bob can be reached at the IP address 192.168.0.10 at port 5060 (the default SIP port). This private IP address is meaningless to a device on the public Internet and the registrar would not know how to reach Bob.

A second example involves problems in sending RTP media. Alice calls Bob and Alice’s invite contains SDP with her local IP address 10.1.1.10 and media port 1234. Bob accepts Alice’s invite with his SDP containing his local IP address 192.168.0.10 and media port 1234. Both of these IP addresses are meaningless outside the scope of each individual’s private local network and neither party will receive the other’s RTP packets.

In these examples, both Alice and Bob would set up an IP address and port mapping with a TURN server. Alice and Bob would then be able to communicate with each other using the TURN server as an intermediary.


TCP TURN Control Connection
As with any TURN negotiation, the first step of the process is to create an allocation. When using TCP TURN it is necessary that the transport type of the allocation be that of TCP. Although a UDP allocation can be created by using a TCP transport, this property is not reciprocal. In addition, UDP based attributes such as tokens, and even-ports, must be left out of all TCP allocation requests. TCP allocation authentication, success/failure responses, and the creation of permissions are identical to those of UDP TURN. It is important however to identify this allocated connection as a “Control Connection,” as it serves a specific purpose in TCP applications.

TCP TURN Data Connection

At this point, the control connection allocation and permissions have been established with the TURN server, the next step is to create the “data connection” where most of the information transaction will take place. In UDP applications this would normally just take place on the already established connection as long as the proper permissions were in place. With TCP TURN an additional step is necessary.

When creating a connection as the establisher, a Connection request is sent to the server over the control connection. This Connect request must contain the XOR address of the peer with which to establish a connection. If everything is good, and the peer has created the proper permissions on their end to accept contact, the server will send back a success response containing the stun attribute “connectionID.”

Once this response is received, it is necessary to open a new connection with the server using a different local port, but sending data to the same established socket on the TURN server. Once the port is open, a “connectionBind” request can be sent over this new pathway. The connectionBind request must contain the previously sent connectionID, XOR peer address, and any of the negotiated authentications of the established control connection. The inclusion of these attributes greatly reduces the chances of hijacking and improves the security of the established data connection.

Once a success response is received, data may flow over this connection in a normal manner, if additional media types are required, a data connection for each must be established in an appropriate manner adhering to the procedure above. If receiving a connection from the client, the method is the same, but using a “connectionAttempt” to start the process off. As long as a permission is in place for the inquiring address, a connectionBind request will be sent back to the server and a data connection established.

Once both the Control Connection and Data Connections have been established for both client and peer, data may flow from one data connection to the other. All TURN based messaging, such as refreshes, must be sent on the Control Connection. If at any point the client or peer closes the control connection, the related data connection must be closed as well.
