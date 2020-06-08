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
