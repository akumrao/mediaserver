{
  "webrtc": {
    "dtlsCertificateFile": "/var/tmp/key/certificate.crt",
    "dtlsPrivateKeyFile": "/var/tmp/key/private_key.pem",
    "rtcMinPort": 11501,
    "rtcMaxPort": 12560,
     "rtsp1": "rtsp://root:60056006@10.86.8.16:560/axis-media/media.amp?videocodec=h264&resolution=768x576&fps=25",
    "rtsp2": "rtsp://localhost:8554/testStream",
    "logLevel": "info",
    "logTags": [
      "info"
    ],
    "listenIps": [
      {
        "ip": "0.0.0.0",
        "announcedIp": "52.5.6.245"
      }
    ]

  }
}
