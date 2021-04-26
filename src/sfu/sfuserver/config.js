{
  "webrtc": {
    "dtlsCertificateFile": "/var/tmp/key/certificate.crt",
    "dtlsPrivateKeyFile": "/var/tmp/key/private_key.pem",
    "rtcMinPort": 11501,
    "rtcMaxPort": 12560,
    "logLevel": "info",
    "logTags": [
      "info",
      "ice",
      "dtls",
      "rtp",
      "srtp",
      "rtcp",
      "rtx",
      "bwe",
      "score",
      "simulcast",
      "svc"
    ],
    "listenIps": [
      {
        "ip": "0.0.0.0",
        "announcedIp": "192.168.0.19"
      }
    ],
    "routerCapabilities": {
      "codecs": [
        {
          "kind": "video",
          "mimeType": "video/multiplex",
          "clockRate": 90000,
          "preferredPayloadType": 96,
          "parameters": {
            "acn": "H264",
            "packetization-mode": 1,
            "level-asymmetry-allowed": 1,
            "profile-level-id": "42e01f"
          },
          "rtcpFeedback": [
            {
              "type": "nack"
            },
            {
              "type": "nack",
              "parameter": "pli"
            },
            {
              "type": "ccm",
              "parameter": "fir"
            },
            {
              "type": "goog-remb"
            },
            {
              "type": "transport-cc"
            }
          ]
        },
        {
          "kind": "video",
          "mimeType": "video/H264",
          "clockRate": 90000,
          "preferredPayloadType": 96,
          "parameters": {
            "packetization-mode": 1,
            "level-asymmetry-allowed": 1,
            "profile-level-id": "42e01f"
          },
          "rtcpFeedback": [
            {
              "type": "nack"
            },
            {
              "type": "nack",
              "parameter": "pli"
            },
            {
              "type": "ccm",
              "parameter": "fir"
            },
            {
              "type": "goog-remb"
            },
            {
              "type": "transport-cc"
            }
          ]
        },
        {
          "kind": "audio",
          "mimeType": "audio/opus",
          "clockRate": 48000,
          "channels": 2,
          "rtcpFeedback": [
            {
              "type": "transport-cc"
            }
          ],
          "preferredPayloadType": 100,
          "parameters": {}
        },
        {
          "kind": "audio",
          "mimeType": "audio/PCMU",
          "preferredPayloadType": 0,
          "clockRate": 8000,
          "rtcpFeedback": [
            {
              "type": "transport-cc"
            }
          ]
        },
        {
          "kind": "audio",
          "mimeType": "audio/PCMA",
          "preferredPayloadType": 8,
          "clockRate": 8000,
          "rtcpFeedback": [
            {
              "type": "transport-cc"
            }
          ]
        },
        {
          "kind": "audio",
          "mimeType": "audio/ISAC",
          "clockRate": 32000,
          "preferredPayloadType": 104,
          "rtcpFeedback": [
            {
              "type": "transport-cc"
            }
          ]
        },
        {
          "kind": "audio",
          "mimeType": "audio/ISAC",
          "clockRate": 16000,
          "preferredPayloadType": 103,
          "rtcpFeedback": [
            {
              "type": "transport-cc"
            }
          ]
        },
        {
          "kind": "audio",
          "mimeType": "audio/G722",
          "preferredPayloadType": 9,
          "clockRate": 8000,
          "rtcpFeedback": [
            {
              "type": "transport-cc"
            }
          ]
        },
        {
          "kind": "audio",
          "mimeType": "audio/iLBC",
          "clockRate": 8000,
          "preferredPayloadType": 102,
          "rtcpFeedback": [
            {
              "type": "transport-cc"
            }
          ]
        },

        {
          "kind": "audio",
          "mimeType": "audio/CN",
          "preferredPayloadType": 106,
          "clockRate": 32000
        },
        {
          "kind": "audio",
          "mimeType": "audio/CN",
          "preferredPayloadType": 105,
          "clockRate": 16000
        },
        {
          "kind": "audio",
          "mimeType": "audio/CN",
          "preferredPayloadType": 13,
          "clockRate": 8000
        },
        {
          "kind": "video",
          "mimeType": "video/VP8",
          "clockRate": 90000,
          "rtcpFeedback": [
            {
              "type": "nack"
            },
            {
              "type": "nack",
              "parameter": "pli"
            },
            {
              "type": "ccm",
              "parameter": "fir"
            },
            {
              "type": "goog-remb"
            },
            {
              "type": "transport-cc"
            }
          ],
          "preferredPayloadType": 101,
          "parameters": {
            "x-google-start-bitrate": 1000
          }
        },
        {
          "kind": "video",
          "mimeType": "video/rtx",
          "preferredPayloadType": 102,
          "clockRate": 90000,
          "rtcpFeedback": [],
          "parameters": {
            "apt": 101
          }
        }
      ],
      "headerExtensions": [
        {
          "kind": "audio",
          "uri": "urn:ietf:params:rtp-hdrext:sdes:mid",
          "preferredId": 1,
          "preferredEncrypt": false,
          "direction": "recvonly"
        },
        {
          "kind": "video",
          "uri": "urn:ietf:params:rtp-hdrext:sdes:mid",
          "preferredId": 1,
          "preferredEncrypt": false,
          "direction": "recvonly"
        },
        {
          "kind": "video",
          "uri": "urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id",
          "preferredId": 2,
          "preferredEncrypt": false,
          "direction": "recvonly"
        },
        {
          "kind": "video",
          "uri": "urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id",
          "preferredId": 3,
          "preferredEncrypt": false,
          "direction": "recvonly"
        },
        {
          "kind": "audio",
          "uri": "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time",
          "preferredId": 4,
          "preferredEncrypt": false,
          "direction": "sendrecv"
        },
        {
          "kind": "video",
          "uri": "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time",
          "preferredId": 4,
          "preferredEncrypt": false,
          "direction": "sendrecv"
        },
        {
          "kind": "audio",
          "uri": "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01",
          "preferredId": 5,
          "preferredEncrypt": false,
          "direction": "recvonly"
        },
        {
          "kind": "video",
          "uri": "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01",
          "preferredId": 5,
          "preferredEncrypt": false,
          "direction": "sendrecv"
        },
        {
          "kind": "video",
          "uri": "http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07",
          "preferredId": 6,
          "preferredEncrypt": false,
          "direction": "sendrecv"
        },
        {
          "kind": "video",
          "uri": "urn:ietf:params:rtp-hdrext:framemarking",
          "preferredId": 7,
          "preferredEncrypt": false,
          "direction": "sendrecv"
        },
        {
          "kind": "audio",
          "uri": "urn:ietf:params:rtp-hdrext:ssrc-audio-level",
          "preferredId": 10,
          "preferredEncrypt": false,
          "direction": "sendrecv"
        },
        {
          "kind": "video",
          "uri": "urn:3gpp:video-orientation",
          "preferredId": 11,
          "preferredEncrypt": false,
          "direction": "sendrecv"
        },
        {
          "kind": "video",
          "uri": "urn:ietf:params:rtp-hdrext:toffset",
          "preferredId": 12,
          "preferredEncrypt": false,
          "direction": "sendrecv"
        }
      ],
      "fecMechanisms": []
    },
    "worker_createRouter": {
      "id": 1,
      "method": "worker.createRouter",
      "internal": {
        "routerId": "2e32062d-f04a-4c2d-a656-b586e50498ef"
      }
    },
    "router_createAudioLevelObserver": {
      "id": 2,
      "method": "router.createAudioLevelObserver",
      "internal": {
        "routerId": "3b1e0f99-b122-45f7-a82e-86e634a0f7de",
        "rtpObserverId": "bfbdb9b3-f43f-4250-b362-b290638a6880"
      },
      "data": {
        "maxEntries": 10,
        "threshold": -70,
        "interval": 2000
      }
    },
    "createWebRtcTransport": {
      "id": 3,
      "method": "router.createWebRtcTransport",
      "internal": {
        "routerId": "2e32062d-f04a-4c2d-a656-b586e50498ef",
        "transportId": "e5302612-283c-4532-8acb-8f3cbb87a8a5"
      },
      "data": {
        "listenIps": [
          {
            "ip": "127.0.0.1"
          }
        ],
        "enableUdp": true,
        "enableTcp": true,
        "preferUdp": true,
        "preferTcp": false,
        "initialAvailableOutgoingBitrate": 1000000,
        "enableSctp": false,
        "numSctpStreams": {
          "OS": 1024,
          "MIS": 1024
        },
        "maxSctpMessageSize": 262144,
        "isDataChannel": true
      }
    },
    "maxbitrate": {
      "id": 4,
      "method": "transport.setMaxIncomingBitrate",
      "internal": {
        "routerId": "2e32062d-f04a-4c2d-a656-b586e50498ef",
        "transportId": "e5302612-283c-4532-8acb-8f3cbb87a8a5"
      },
      "data": {
        "bitrate": 1500000
      }
    },
    "transport_connect": {
      "id": 5,
      "method": "transport.connect",
      "internal": {
        "routerId": "2e32062d-f04a-4c2d-a656-b586e50498ef",
        "transportId": "e5302612-283c-4532-8acb-8f3cbb87a8a5"
      },
      "data": {
        "dtlsParameters": {
          "role": "server",
          "fingerprints": [
            {
              "algorithm": "sha-256",
              "value": "30:D3:F2:7C:DB:12:F3:FD:D4:38:31:19:2F:48:B5:ED:85:59:85:99:D2:5C:E8:A5:AE:A2:57:C6:FF:93:57:65"
            }
          ]
        }
      }
    },
    "transport_produce": {
      "id": 6,
      "method": "transport.produce",
      "internal": {
        "routerId": "2e32062d-f04a-4c2d-a656-b586e50498ef",
        "transportId": "e5302612-283c-4532-8acb-8f3cbb87a8a5",
        "producerId": "87c81cfc-b307-4270-a213-4f17e4776931"
      },
      "data": {
        "kind": "video",
        "rtpParameters": {
          "mid": "0",
          "codecs": [
            {
              "mimeType": "video/VP8",
              "clockRate": 90000,
              "payloadType": 96,
              "rtcpFeedback": [
                {
                  "type": "goog-remb"
                },
                {
                  "type": "transport-cc"
                },
                {
                  "type": "ccm",
                  "parameter": "fir"
                },
                {
                  "type": "nack"
                },
                {
                  "type": "nack",
                  "parameter": "pli"
                }
              ],
              "parameters": {}
            },
            {
              "mimeType": "video/rtx",
              "clockRate": 90000,
              "payloadType": 97,
              "rtcpFeedback": [],
              "parameters": {
                "apt": 96
              }
            }
          ],
          "headerExtensions": [
            {
              "uri": "urn:ietf:params:rtp-hdrext:sdes:mid",
              "id": 4
            },
            {
              "uri": "urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id",
              "id": 5
            },
            {
              "uri": "urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id",
              "id": 6
            },
            {
              "uri": "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time",
              "id": 2
            },
            {
              "uri": "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01",
              "id": 3
            },
            {
              "uri": "http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07",
              "id": 8
            },
            {
              "uri": "urn:3gpp:video-orientation",
              "id": 13
            },
            {
              "uri": "urn:ietf:params:rtp-hdrext:toffset",
              "id": 14
            }
          ],
          "encodings": [
            {
              "ssrc": 6897981,
              "rtx": {
                "ssrc": 3637419867
              }
            }
          ],
          "rtcp": {
            "cname": "d1blHuW4nPAoUtQb"
          }
        },
        "rtpMapping": {
          "codecs": [
            {
              "payloadType": 96,
              "mappedPayloadType": 101
            },
            {
              "payloadType": 97,
              "mappedPayloadType": 102
            }
          ],
          "encodings": [
            {
              "mappedSsrc": 632915476,
              "ssrc": 6897981
            }
          ]
        },
        "paused": false
      }
    },
    "rtpObserver_addProducer": {
      "id": 7,
      "method": "rtpObserver.addProducer",
      "internal": {
        "routerId": "3b1e0f99-b122-45f7-a82e-86e634a0f7de",
        "rtpObserverId": "bfbdb9b3-f43f-4250-b362-b290638a6880",
        "producerId": "19bbfa3f-50dc-460c-8684-3c7fb39ce0fd"
      }
    },
    "transport_consume": {
      "id": 8,
      "method": "transport.consume",
      "internal": {
        "routerId": "2e32062d-f04a-4c2d-a656-b586e50498ef",
        "transportId": "4ca62904-639c-4b5a-86e8-f0bc84bfe776",
        "consumerId": "c3dbd428-ff6e-46b6-a1e7-ba3891b70f34",
        "producerId": "87c81cfc-b307-4270-a213-4f17e4776931"
      },
      "data": {
        "kind": "video",
        "rtpParameters": {
          "codecs": [
            {
              "mimeType": "video/VP8",
              "clockRate": 90000,
              "payloadType": 101,
              "rtcpFeedback": [
                {
                  "type": "transport-cc"
                },
                {
                  "type": "ccm",
                  "parameter": "fir"
                },
                {
                  "type": "nack"
                },
                {
                  "type": "nack",
                  "parameter": "pli"
                }
              ],
              "parameters": {}
            },
            {
              "mimeType": "video/rtx",
              "clockRate": 90000,
              "payloadType": 102,
              "rtcpFeedback": [],
              "parameters": {
                "apt": 101
              }
            }
          ],
          "headerExtensions": [
            {
              "uri": "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time",
              "id": 4
            },
            {
              "uri": "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01",
              "id": 5
            },
            {
              "uri": "http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07",
              "id": 6
            },
            {
              "uri": "urn:3gpp:video-orientation",
              "id": 11
            },
            {
              "uri": "urn:ietf:params:rtp-hdrext:toffset",
              "id": 12
            }
          ],
          "encodings": [
            {
              "ssrc": 685058887,
              "rtx": {
                "ssrc": 463150472
              }
            }
          ],
          "rtcp": {
            "cname": "d1blHuW4nPAoUtQb",
            "reducedSize": true,
            "mux": true
          }
        },
        "type": "simple",
        "consumableRtpEncodings": [
          {
            "ssrc": 632915476
          }
        ],
        "paused": true
      }
    },
    "pause_resume": {
      "id": 9,
      "method": "consumer.resume",
      "internal": {
        "routerId": "2e32062d-f04a-4c2d-a656-b586e50498ef",
        "transportId": "4ca62904-639c-4b5a-86e8-f0bc84bfe776"
      }
    },
    "producer_getStats": {
      "id": 10,
      "method": "producer.getStats",
      "internal": {
        "routerId": "3b1e0f99-b122-45f7-a82e-86e634a0f7de",
        "transportId": "a59321ad-62ca-4832-87a8-c9e83399ff95",
        "producerId": "f177c5ad-8a15-4631-89a8-43f0ae061a7c"
      }
    },
    "consumer_getStats": {
      "id": 40,
      "method": "consumer.getStats",
      "internal": {
        "routerId": "48584627-a161-42d2-83bd-dd578ef4dd3a",
        "transportId": "bd2cab3d-bb60-4734-a9ba-520f46de5d5f",
        "consumerId": "0bcf410e-7562-46a0-ae1a-8ec4068edaa9",
        "producerId": "225a07bd-6478-460a-a975-3a920aa3d329"
      }
    },
    "consumer_setPreferredLayers": {
      "id": 41,
      "method": "consumer.setPreferredLayers",
      "internal": {
        "routerId": "48584627-a161-42d2-83bd-dd578ef4dd3a",
        "transportId": "bd2cab3d-bb60-4734-a9ba-520f46de5d5f",
        "consumerId": "0bcf410e-7562-46a0-ae1a-8ec4068edaa9",
        "producerId": "225a07bd-6478-460a-a975-3a920aa3d329"
      },
      "data": {
        "spatialLayer": 0
      }
    },
    "transport_enableTraceEvent": {
      "id": 42,
      "method": "transport.enableTraceEvent",
      "internal": {
        "routerId": "2e32062d-f04a-4c2d-a656-b586e50498ef",
        "transportId": "e5302612-283c-4532-8acb-8f3cbb87a8a5"
      },
      "data": {
        "types":["bwe"]
      }
    },
   "transport_close": {
      "id": 45,
      "method": "transport.close",
      "internal": {
        "routerId": "2e32062d-f04a-4c2d-a656-b586e50498ef",
        "transportId": "e5302612-283c-4532-8acb-8f3cbb87a8a5"
      }
    }


  }
}
