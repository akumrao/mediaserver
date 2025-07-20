{
    "dtlsCertificateFile": "/var/tmp/key/certificate.crt",
    "dtlsPrivateKeyFile": "/var/tmp/key/private_key.pem",
    "storage": "/media/pvi-storage/",
    "listenIps": [
        {
            "announcedIp": "52.5.6.245",
            "ip": "0.0.0.0"
        }
    ],
    "logLevel": "info",
    "logTags": [
        "info"
    ],
    "potentialID": "729795",
    "rtcMaxPort": 12560,
    "rtcMinPort": 11501,
    "rtsp": {
        "41": {
             "recording": "on",
            "rtsp": "rtsp://localhost/test.264",
            "state": "streaming"
        },
        "42": {
            "recording": "on",
            "rtsp": "rtsp://root:herman@192.168.0.90/axis-media/media.amp",
            "state": "streaming"
        },
        "529795F1234CAM14562": {
            "rtsp": "rtsp://root:001o000000vmbHtAAI@10.86.12.100:558/axis-media/media.amp?videocodec=h264&resolution=768x576&fps=25"
        },
        
        "629795F1234CAM14569": {
            "rtsp": "rtsp://root:001o000000vmbHtAAI@10.86.12.100:554/axis-media/media.amp?videocodec=h264&resolution=768x576&fps=25"
        },
        "729795F1234CAM14528": {
            "rtsp": "rtsp://root:60056005@10.76.0.16:555/axis-media/media.amp?videocodec=h264&resolution=768x576&fps=25"
        },
        "cam09": {
            "rtsp": "rtsp://root:0011J00001kV6yjQAC@10.86.7.224:559/axis-media/media.amp?videocodec=h264&resolution=1280x720&fps=5"
        }
    },
    "stream_type": "webrtc"
}
