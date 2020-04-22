module.exports = {
  listenIp: '0.0.0.0',
  listenPort: 8080,
  sslCrt: '/etc/ssl/certs/ssl-cert-snakeoil.pem',
  sslKey: '/etc/ssl/private/ssl-cert-snakeoil.key',

  webRtcTransport: {
      listenIps: [
        {
          ip: '127.0.0.1'
        }
      ],
      maxIncomingBitrate: 1500000,
      initialAvailableOutgoingBitrate: 1000000,
    }
};
