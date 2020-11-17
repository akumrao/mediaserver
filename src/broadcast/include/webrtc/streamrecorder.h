
#ifndef WebRTC_StreamRecorder_H
#define WebRTC_StreamRecorder_H


#include "base/base.h"

#ifdef HAVE_FFMPEG

#include "av/av.h"
#include "av/multiplexencoder.h"

#include "api/peerconnectioninterface.h"


namespace base {
namespace wrtc {


class StreamRecorder : public rtc::VideoSinkInterface<webrtc::VideoFrame>,
                       public webrtc::AudioTrackSinkInterface
{
public:
    StreamRecorder(const av::EncoderOptions& options);
    ~StreamRecorder();

    void setVideoTrack(webrtc::VideoTrackInterface* track);
    void setAudioTrack(webrtc::AudioTrackInterface* track);

    /// VideoSinkInterface implementation
    void OnFrame(const webrtc::VideoFrame& frame) override;

    /// AudioTrackSinkInterface implementation
    void OnData(const void* audio_data, int bits_per_sample, int sample_rate,
                size_t number_of_channels, size_t number_of_frames) override;

protected:
    av::MultiplexEncoder _encoder;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> _videoTrack;
    rtc::scoped_refptr<webrtc::AudioTrackInterface> _audioTrack;
    bool _awaitingVideo = false;
    bool _awaitingAudio = false;
    bool _shouldInit = true;
};


} } // namespace wrtc


#endif // HAVE_FFMPEG
#endif


