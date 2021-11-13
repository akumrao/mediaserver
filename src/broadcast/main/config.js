{
  "webrtc": {
    "dtlsCertificateFile": "/var/tmp/key/certificate.crt",
    "dtlsPrivateKeyFile": "/var/tmp/key/private_key.pem",
    "rtcMinPort": 11501,
    "rtcMaxPort": 12560,
      "rtsp":{
      "cam0":"rtsp://root:60056006@10.86.8.16:560/axis-media/media.amp?videocodec=h264&resolution=768x576&fps=25",
      "cam1":"rtsp://admin:247Supp0rt!@10.86.7.224:557/RTSP2HLS/media.smp",
      "cam2":"rtsp://root:60056006@10.86.8.16:606/axis-media/media.amp",
      "cam3":"rtsp://localhost:8554/testStream",
      "cam4":"rtsp://localhost:9554/testStream"
    },
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
