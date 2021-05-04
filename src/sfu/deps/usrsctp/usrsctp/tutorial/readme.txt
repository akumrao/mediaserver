To compile this code, sctp dev libraries are needed . 
For ubuntu : sudo apt-get install libsctp-dev 

gcc -g sctpserver.c -o server  -lsctp
 gcc -g sctpclt.c -o client -lsctp

 netstat --sctp -tulpn


 $ cat /proc/net/sctp/eps
 ENDPT     SOCK   STY SST HBKT LPORT   UID INODE LADDRS
b6d72780 a8903800 2   10  48   123       0 1895802 0.0.0.0
Take that inode (1895802 in my example) and use lsof to find who owns it:

$ lsof -R | grep 1895802


Above example is not used at webrtc 

sctp over udp encapsulation 

Socket (port 5000)
IP and por 


 SCTP over UDP (in which ports 5000 mean nothing, it’s just cosmetic and required by mediasoup PlainTransport to properly demux SCTP packets received over UDP).


 I’m definitely using SCTP over UDP, have been the whole time. I can connect to a node-sctp socket with udp transport from usrsctp, and send messages back and forth fine, I just can’t connect to mediasoup’s transport. Here’s a gist of what’s not working, perhaps I’m missing a socket option or a parameter is wrong (AF_INET vs AF_CONN??)

Mediasoup server:

await dataTransport.connect({ ip: REMOTE_IP, port: 44443 });
usrsctp client:







usrsctp_init(44443, NULL, NULL); // Use UDP port 44443 for SCTP
struct socket* sctp_socket = usrsctp_socket(AF_CONN, SOCK_STREAM, IPPROTO_SCTP, NULL, NULL, 0, NULL);
local.sin_addr.s_addr = htonl(INADDR_ANY);
local.sin_port = htons(5000);
dest.sin_addr.s_addr = inet_addr(MEDIASOUP_SERVER_IP);
dest.sin_port = htons(5000);
// Use the Mediasoup transport endpoint localPort as the remote UDP port for SCTP over UDP.
encaps.sue_port = htons(MEDIASOUP_TRANPORT_PORT);
usrsctp_setsockopt(sctp_socket, IPPROTO_SCTP, SCTP_REMOTE_UDP_ENCAPS_PORT,
                   (const void*)&encaps, (socklen_t)sizeof(struct sctp_udpencaps);
usrsctp_bind(sctp_socket, (struct sockaddr*)&local, sizeof(struct sockaddr_in));
usrsctp_connect(sctp_socket, (struct sockaddr*)&dest, sizeof(struct sockaddr_in));






or check below code


  const sendTransport = await router.createPlainRtpTransport({
    listenIp: { ip: SRC_IP },
    enableSctp: true,
    numSctpStreams: { OS: 512, MIS: 512 },
    maxSctpMessageSize: 4000000 // 4 MB
  });

  // Node UDP socket for the sending SCTP.
  const sendUdpSocket = dgram.createSocket({ type: 'udp4' });

  await new Promise(resolve => sendUdpSocket.bind(11111, SRC_IP, resolve));

  const localSendUdpPort = sendUdpSocket.address().port;

  // Connect the mediasoup send PlainRtpTransport to the UDP socket.
  await sendTransport.connect({ ip: SRC_IP, port: localSendUdpPort });

  // Create a node-sctp sending Socket.
  const sendSctpSocket = sctp.connect({
    localPort: 5000, // Required for SCTP over plain UDP in mediasoup.
    port: 5000,      // Required for SCTP over plain UDP in mediasoup.
    OS: sendTransport.sctpParameters.OS,
    MIS: sendTransport.sctpParameters.MIS,
    unordered: false,
    udpTransport: sendUdpSocket,
    udpPeer: {
      address: sendTransport.tuple.localIp,
      port: sendTransport.tuple.localPort,
    },
    highWaterMark: HIGH_WATER_MARK
  });

  sendSctpSocket.on('error', (error) => {
    console.error('[ERROR] node-sctp sending Socket "error" event: %o', error);

    process.exit(2);
  });

  console.log('[INFO] waiting for the sending SCTP association to be open');

  // Wait for the SCTP association to be open.
  await Promise.race([
    new Promise((resolve, reject) => {
      setTimeout(() => reject(new Error('SCTP connection timeout')), 2000)
    }),
    new Promise(resolve => sendSctpSocket.on('connect', resolve)),
  ]);

  console.log('[INFO] creating a node-sctp outbound Stream [streamId:1]');

  // Create a node-sctp outbound Stream with id 1 (don't use the implicit Stream in the
  // node-sctp Socket).
  const outboundSctpStream = sendSctpSocket.createStream(1);

  // Create a mediasoup DataProducer representing the node-sctp outbound Stream.
  console.log(
    '[INFO] creating a mediasoup DataProducer associated to the node-sctp outbound Stream');

  const dataProducer = await sendTransport.produceData({
    sctpStreamParameters: {
      streamId: 1,
      ordered: true,
    }
  });


