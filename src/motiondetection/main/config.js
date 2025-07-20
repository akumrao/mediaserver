{
  "dtlsCertificateFile": "/var/tmp/key/certificate.crt",
  "dtlsPrivateKeyFile": "/var/tmp/key/private_key.pem",
  "storage": "/media/pvi-storage/",
  "logLevel": "info",
  "potentialID": "729795",
  "pre_clip": 5,
  "cam_reconnect": 0,
  "rtsp": {
    "Cam04": {
      "rtsp": "rtsp://192.168.0.19/test12.264",
      "CLIPSIZE": 20,
       "contourArea": 8,
       "DEFAULT_WINDOW_SIZE": 2,
      "soft2minRule": {
        "contourArea": 300,
        "DEFAULT_WINDOW_SIZE": 3
      },
      "nonMonitoringHr": {
        "statrtHr": 18,
        "endHr": 6,
        "statrtMin": 30,
        "endMin": 30,
        "contourArea": 300,
        "DEFAULT_WINDOW_SIZE": 3
      },
      "md": true,
      "masking_coords": [
        [
          {
            "x": 0,
            "y": 0
          },
          {
            "x": 30,
            "y": 0
          },
          {
            "x": 30,
            "y": 220
          },
          {
            "x": 0,
            "y": 220
          }
        ]
      ]
    },
    "Cam12": {
      "rtsp": "rtsp://localhost/test13.264",
      "CLIPSIZE": 20,
      "default": {
        "contourArea": 200,
        "DEFAULT_WINDOW_SIZE": 2
      },
      "md": true
    }
  },
  "stream_type": "mdserver"
}
