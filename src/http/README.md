Websocket


**Websocket spec.**

https://tools.ietf.org/id/draft-ietf-hybi-thewebsocketprotocol-09.html#rfc.section.4.2





hixie-75  February 4, 2010        4 5.0.0   
hixie-76
hybi-00 May 6, 2010
May 23, 2010    4.0 (disabled)    6 5.0.1 11.00 (disabled)  
hybi-07, v7 April 22, 2011    6[21][a]          
hybi-10, v8 July 11, 2011   7[23][a]  7 14[24]      
RFC 6455, v13 December, 2011  10[25]  11  11  16[26]  6 12.10[27] 4.4




GET /chat HTTP/1.1
Host: server.example.com
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==
Sec-WebSocket-Protocol: chat, superchat
Sec-WebSocket-Version: 13
Origin: http://example.com
Server response:

HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: HSmrc0sMlYUkAGmm5OPpG2HaGWk=
Sec-WebSocket-Protocol: chat

https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_servers
https://github.com/katzarsky/WebSocket/blob/master/WebSocket/WebSocket.cpp



Message Fragmentation
The FIN and opcode fields work together to send a message split up into separate frames.  This is called message fragmentation. Fragmentation is only available on opcodes 0x0 to 0x2.

Recall that the opcode tells what a frame is meant to do. If it's 0x1, the payload is text. If it's 0x2, the payload is binary data. However, if it's 0x0, the frame is a continuation frame; this means the server should concatenate the frame's payload to the last frame it received from that client.Here is a rough sketch, in which a server reacts to a client sending text messages. The first message is sent in a single frame, while the second message is sent across three frames. FIN and opcode details are shown only for the client:

Client: FIN=1, opcode=0x1, msg="hello"
Server: (process complete message immediately) Hi.
Client: FIN=0, opcode=0x1, msg="and a"
Server: (listening, new message containing text started)
Client: FIN=0, opcode=0x0, msg="happy new"
Server: (listening, payload concatenated to previous message)
Client: FIN=1, opcode=0x0, msg="year!"
Server: (process complete message) Happy new year to you too!
Notice the first frame contains an entire message (has FIN=1 and opcode!=0x0), so the server can process or respond as it sees fit. The second frame sent by the client has a text payload (opcode=0x1), but the entire message has not arrived yet (FIN=0). All remaining parts of that message are sent with continuation frames (opcode=0x0), and the final frame of the message is marked by FIN=1. Section 5.4 of the spec describes message fragmentation.



https://stackoverflow.com/questions/18368130/how-to-parse-and-validate-a-websocket-frame-in-java

Things you can validate:

https://datatracker.ietf.org/doc/html/rfc6455#section-5.2

Is the opcode found one of the valid ones defined in the spec?
Are RSV bits being used improperly?



If Frame was sent by Client, is the Frame Masked?
Is the Mask random from frame to frame?
Don't allow [0x00, 0x00, 0x00, 0x00] as a mask.
Fragmentation: RFC 6455 - Section 5.4

Is it a fragmented Control frame?
Is the fragmentation of a large message, consisting of multiple frames, out of order?
Was a new message started before prior one completed with a FIN flag?
Control Frames: RFC 6455 - Section 5.5

Does the payload length of a control frame exceed 125 bytes?
Is the payload fragmented?
Close Frames: RFC 6455 - Section 5.5.1

If a status code is provided in the payload, does the status code conform to one of the status codes declared in section 7.4.1? Don't forget to to check the IANA registry of websocket status codes that were added after the RFC was finalized)
Is the status code one that is allowed to be sent over the network in a Frame? (see codes 1005, and 1006 for example)
If a /reason/ is provided in the frame, does it conform to UTF-8 encoding rules?
Have you received any frames, of any kind, after a Close frame? (this is a no-no)
Data Frames: RFC 6455 - Section 5.6

If you receive a TEXT payload data (from TEXT + CONTINUATION frames), does the payload data conform to UTF-8 encoding rules?
While you can validate at the individual frame level, you will find that some of the validations above are validations of state and behavior between multiple frames. You can find more of these kinds of validations in Sending and Receiving Data: RFC 6455 - Section 6.

However, if you have extensions in the mix, then you will also need to process the frames from the point of view of the negotiated extension stack as well. Some tests above would appear to be invalid when an extension is being used.

Example: You have Compression Extension (RFC-7692) (such as permessage-deflate) in use, then the validation of TEXT payload cannot be done with the raw frame off the network, as you must first pass the frame through the extension. Note that the extension can change the fragmentation to suit its needs, which might mess up your validation as well.

package websocket;

import java.nio.ByteBuffer;
import java.nio.charset.Charset;

public class RawParse
{
    public static class Frame
    {
        byte opcode;
        boolean fin;
        byte payload[];
    }

    public static Frame parse(byte raw[])
    {
        // easier to do this via ByteBuffer
        ByteBuffer buf = ByteBuffer.wrap(raw);

        // Fin + RSV + OpCode byte
        Frame frame = new Frame();
        byte b = buf.get();
        frame.fin = ((b & 0x80) != 0);
        boolean rsv1 = ((b & 0x40) != 0);
        boolean rsv2 = ((b & 0x20) != 0);
        boolean rsv3 = ((b & 0x10) != 0);
        frame.opcode = (byte)(b & 0x0F);

        // TODO: add control frame fin validation here
        // TODO: add frame RSV validation here

        // Masked + Payload Length
        b = buf.get();
        boolean masked = ((b & 0x80) != 0);
        int payloadLength = (byte)(0x7F & b);
        int byteCount = 0;
        if (payloadLength == 0













    This wire format for the data transfer part is described by the ABNF
   [RFC5234] given in detail in this section.  (Note that, unlike in
   other sections of this document, the ABNF in this section is
   operating on groups of bits.  The length of each group of bits is
   indicated in a comment.  When encoded on the wire, the most
   significant bit is the leftmost in the ABNF).  A high-level overview
   of the framing is given in the following figure.  In a case of
   conflict between the figure below and the ABNF specified later in
   this section, the figure is authoritative.

      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+

   FIN:  1 bit

      Indicates that this is the final fragment in a message.  The first
      fragment MAY also be the final fragment.

   RSV1, RSV2, RSV3:  1 bit each

      MUST be 0 unless an extension is negotiated that defines meanings
      for non-zero values.  If a nonzero value is received and none of
      the negotiated extensions defines the meaning of such a nonzero
      value, the receiving endpoint MUST _Fail the WebSocket
      Connection_.




Fette & Melnikov             Standards Track                   [Page 28]

RFC 6455                 The WebSocket Protocol            December 2011


   Opcode:  4 bits

      Defines the interpretation of the "Payload data".  If an unknown
      opcode is received, the receiving endpoint MUST _Fail the
      WebSocket Connection_.  The following values are defined.

      *  %x0 denotes a continuation frame

      *  %x1 denotes a text frame

      *  %x2 denotes a binary frame

      *  %x3-7 are reserved for further non-control frames

      *  %x8 denotes a connection close

      *  %x9 denotes a ping

      *  %xA denotes a pong

      *  %xB-F are reserved for further control frames

   Mask:  1 bit

      Defines whether the "Payload data" is masked.  If set to 1, a
      masking key is present in masking-key, and this is used to unmask
      the "Payload data" as per Section 5.3.  All frames sent from
      client to server have this bit set to 1.

   Payload length:  7 bits, 7+16 bits, or 7+64 bits

      The length of the "Payload data", in bytes: if 0-125, that is the
      payload length.  If 126, the following 2 bytes interpreted as a
      16-bit unsigned integer are the payload length.  If 127, the
      following 8 bytes interpreted as a 64-bit unsigned integer (the
      most significant bit MUST be 0) are the payload length.  Multibyte
      length quantities are expressed in network byte order.  Note that
      in all cases, the minimal number of bytes MUST be used to encode
      the length, for example, the length of a 124-byte-long string
      can't be encoded as the sequence 126, 0, 124.  The payload length
      is the length of the "Extension data" + the length of the
      "Application data".  The length of the "Extension data" may be
      zero, in which case the payload length is the length of the
      "Application data".


