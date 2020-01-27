

#ifndef WebRTC_WEBRTC_H
#define WebRTC_WEBRTC_H


namespace base {
namespace wrtc {
// namespace webrtc {


/// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

/// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

/// Labels for ICE streams.
const char kAudioLabel[] = "audio_label";
const char kVideoLabel[] = "video_label";
const char kStreamLabel[] = "stream_label";

/// Server URIs for ICE candidates.
const char kGoogleStunServerUri[] = "stun:stun.l.google.com:19302";


} } // namespace:wrtc


#endif // WebRTC_WEBRTC_H

