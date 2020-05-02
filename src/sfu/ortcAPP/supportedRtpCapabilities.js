"use strict";
console.log("supportedRtpCapabilities");
Object.defineProperty(exports, "__esModule", { value: true });
const supportedRtpCapabilities = {
    codecs: [
        {
            kind: 'audio',
            mimeType: 'audio/opus',
            clockRate: 48000,
            channels: 2,
            rtcpFeedback: [
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'audio',
            mimeType: 'audio/PCMU',
            preferredPayloadType: 0,
            clockRate: 8000,
            rtcpFeedback: [
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'audio',
            mimeType: 'audio/PCMA',
            preferredPayloadType: 8,
            clockRate: 8000,
            rtcpFeedback: [
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'audio',
            mimeType: 'audio/ISAC',
            clockRate: 32000,
            rtcpFeedback: [
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'audio',
            mimeType: 'audio/ISAC',
            clockRate: 16000,
            rtcpFeedback: [
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'audio',
            mimeType: 'audio/G722',
            preferredPayloadType: 9,
            clockRate: 8000,
            rtcpFeedback: [
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'audio',
            mimeType: 'audio/iLBC',
            clockRate: 8000,
            rtcpFeedback: [
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'audio',
            mimeType: 'audio/SILK',
            clockRate: 24000,
            rtcpFeedback: [
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'audio',
            mimeType: 'audio/SILK',
            clockRate: 16000,
            rtcpFeedback: [
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'audio',
            mimeType: 'audio/SILK',
            clockRate: 12000,
            rtcpFeedback: [
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'audio',
            mimeType: 'audio/SILK',
            clockRate: 8000,
            rtcpFeedback: [
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'audio',
            mimeType: 'audio/CN',
            preferredPayloadType: 13,
            clockRate: 32000
        },
        {
            kind: 'audio',
            mimeType: 'audio/CN',
            preferredPayloadType: 13,
            clockRate: 16000
        },
        {
            kind: 'audio',
            mimeType: 'audio/CN',
            preferredPayloadType: 13,
            clockRate: 8000
        },
        {
            kind: 'audio',
            mimeType: 'audio/telephone-event',
            clockRate: 48000
        },
        {
            kind: 'audio',
            mimeType: 'audio/telephone-event',
            clockRate: 32000
        },
        {
            kind: 'audio',
            mimeType: 'audio/telephone-event',
            clockRate: 16000
        },
        {
            kind: 'audio',
            mimeType: 'audio/telephone-event',
            clockRate: 8000
        },
        {
            kind: 'video',
            mimeType: 'video/VP8',
            clockRate: 90000,
            rtcpFeedback: [
                { type: 'nack' },
                { type: 'nack', parameter: 'pli' },
                { type: 'ccm', parameter: 'fir' },
                { type: 'goog-remb' },
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'video',
            mimeType: 'video/VP9',
            clockRate: 90000,
            rtcpFeedback: [
                { type: 'nack' },
                { type: 'nack', parameter: 'pli' },
                { type: 'ccm', parameter: 'fir' },
                { type: 'goog-remb' },
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'video',
            mimeType: 'video/H264',
            clockRate: 90000,
            parameters: {
                'packetization-mode': 1,
                'level-asymmetry-allowed': 1
            },
            rtcpFeedback: [
                { type: 'nack' },
                { type: 'nack', parameter: 'pli' },
                { type: 'ccm', parameter: 'fir' },
                { type: 'goog-remb' },
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'video',
            mimeType: 'video/H264',
            clockRate: 90000,
            parameters: {
                'packetization-mode': 0,
                'level-asymmetry-allowed': 1
            },
            rtcpFeedback: [
                { type: 'nack' },
                { type: 'nack', parameter: 'pli' },
                { type: 'ccm', parameter: 'fir' },
                { type: 'goog-remb' },
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'video',
            mimeType: 'video/H265',
            clockRate: 90000,
            parameters: {
                'packetization-mode': 1,
                'level-asymmetry-allowed': 1
            },
            rtcpFeedback: [
                { type: 'nack' },
                { type: 'nack', parameter: 'pli' },
                { type: 'ccm', parameter: 'fir' },
                { type: 'goog-remb' },
                { type: 'transport-cc' }
            ]
        },
        {
            kind: 'video',
            mimeType: 'video/H265',
            clockRate: 90000,
            parameters: {
                'packetization-mode': 0,
                'level-asymmetry-allowed': 1
            },
            rtcpFeedback: [
                { type: 'nack' },
                { type: 'nack', parameter: 'pli' },
                { type: 'ccm', parameter: 'fir' },
                { type: 'goog-remb' },
                { type: 'transport-cc' }
            ]
        }
    ],
    headerExtensions: [
        {
            kind: 'audio',
            uri: 'urn:ietf:params:rtp-hdrext:sdes:mid',
            preferredId: 1,
            preferredEncrypt: false,
            direction: 'recvonly'
        },
        {
            kind: 'video',
            uri: 'urn:ietf:params:rtp-hdrext:sdes:mid',
            preferredId: 1,
            preferredEncrypt: false,
            direction: 'recvonly'
        },
        {
            kind: 'video',
            uri: 'urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id',
            preferredId: 2,
            preferredEncrypt: false,
            direction: 'recvonly'
        },
        {
            kind: 'video',
            uri: 'urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id',
            preferredId: 3,
            preferredEncrypt: false,
            direction: 'recvonly'
        },
        {
            kind: 'audio',
            uri: 'http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time',
            preferredId: 4,
            preferredEncrypt: false,
            direction: 'sendrecv'
        },
        {
            kind: 'video',
            uri: 'http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time',
            preferredId: 4,
            preferredEncrypt: false,
            direction: 'sendrecv'
        },
        // NOTE: For audio we just enable transport-wide-cc-01 when receiving media.
        {
            kind: 'audio',
            uri: 'http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01',
            preferredId: 5,
            preferredEncrypt: false,
            direction: 'recvonly'
        },
        {
            kind: 'video',
            uri: 'http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01',
            preferredId: 5,
            preferredEncrypt: false,
            direction: 'sendrecv'
        },
        // NOTE: Remove this once framemarking draft becomes RFC.
        {
            kind: 'video',
            uri: 'http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07',
            preferredId: 6,
            preferredEncrypt: false,
            direction: 'sendrecv'
        },
        {
            kind: 'video',
            uri: 'urn:ietf:params:rtp-hdrext:framemarking',
            preferredId: 7,
            preferredEncrypt: false,
            direction: 'sendrecv'
        },
        {
            kind: 'audio',
            uri: 'urn:ietf:params:rtp-hdrext:ssrc-audio-level',
            preferredId: 10,
            preferredEncrypt: false,
            direction: 'sendrecv'
        },
        {
            kind: 'video',
            uri: 'urn:3gpp:video-orientation',
            preferredId: 11,
            preferredEncrypt: false,
            direction: 'sendrecv'
        },
        {
            kind: 'video',
            uri: 'urn:ietf:params:rtp-hdrext:toffset',
            preferredId: 12,
            preferredEncrypt: false,
            direction: 'sendrecv'
        }
    ],
    fecMechanisms: []
};
exports.default = supportedRtpCapabilities;
