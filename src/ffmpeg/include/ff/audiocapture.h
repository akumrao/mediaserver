

#ifndef BASE_AV_AudioCapture_H
#define BASE_AV_AudioCapture_H


#include "base/base.h"

#ifdef HAVE_FFMPEG

#include "ff/mediacapture.h"


namespace base {
namespace ff {

/// This class implements a cross platform audio capturer.
class AudioCapture : public MediaCapture
{
public:
    typedef std::shared_ptr<AudioCapture> Ptr;

    AudioCapture();
    AudioCapture(const std::string& device, const av::AudioCodec& params);
    AudioCapture(const std::string& device, int channels = -1, int sampleRate = -1, 
                 const std::string& sampleFmt = "");
    virtual ~AudioCapture();

    virtual void openAudio(const std::string& device, const av::AudioCodec& params);
    virtual void openAudio(const std::string& device, int channels = -1, int sampleRate = -1, 
                           const std::string& sampleFmt = "");
};


} // namespace ff
} // namespace base


#endif
#endif // BASE_AV_AudioCapture_H


/// @\}
