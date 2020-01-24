

#ifndef BASE_AV_VideoContext_H
#define BASE_AV_VideoContext_H


#include "base/base.h"
//#include "base/packetsignal.h"

#ifdef HAVE_FFMPEG

#include "ff/ffmpeg.h"
#include "ff/format.h"
#include "ff/fpscounter.h"
#include "ff/packet.h"
#include "ff/videoconverter.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
// #include <libavutil/fifo.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}


namespace base {
namespace ff {


/// Base video context from which all video encoders and decoders derive.
struct VideoContext
{
    VideoContext();
    virtual ~VideoContext();

    /// Create the `AVCodecContext` using default values
    virtual void create();

    /// Open the `AVCodecContext`
    virtual void open();

    /// Close the `AVCodecContext`
    virtual void close();

    /// Convert the video frame and return the result.
    ///
    /// The input frame will only be converted if it doesn't match the output
    /// format. If the frame is not converted the input frame will be returned.
    /// If the input frame format does not match the `VideoConverter` context
    /// then the `VideoConverter` will be recreated with the input frame params.
    virtual AVFrame* convert(AVFrame* iframe);

    virtual bool recreateConverter();

//    PacketSignal emitter;

    VideoCodec iparams;   ///< input parameters
    VideoCodec oparams;   ///< output parameters
    AVStream* stream;     ///< encoder or decoder stream
    AVCodecContext* ctx;  ///< encoder or decoder context
    AVCodec* codec;       ///< encoder or decoder codec
    AVFrame* frame;       ///< encoder or decoder frame
    VideoConverter* conv; ///< video conversion context
    // FPSCounter fps;          ///< encoder or decoder fps rate
    // double pts;              ///< pts in decimal seconds
    int64_t time;         ///< stream time in codec time base
    int64_t pts;          ///< last packet pts value
    double seconds;       ///< video time in seconds
    std::string error;    ///< error message
};


//
// Inline helpers
//


AVFrame* createVideoFrame(AVPixelFormat pixelFmt, int width, int height);
AVFrame* cloneVideoFrame(AVFrame* source);
void initVideoCodecFromContext(const AVStream* stream, const AVCodecContext* ctx, VideoCodec& params);
AVPixelFormat selectPixelFormat(AVCodec* codec, VideoCodec& params);



} // namespace ff
} // namespace base


#endif
#endif // BASE_AV_VideoContext_H


