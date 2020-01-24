

#ifndef BASE_AV_VideoConverter_H
#define BASE_AV_VideoConverter_H


#include "base/base.h"

#ifdef HAVE_FFMPEG

#include "ff/packet.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}


namespace base {
namespace ff {


struct VideoConverter
{
    VideoConverter();
    virtual ~VideoConverter();

    virtual void create();
    virtual void close();

    virtual AVFrame* convert(AVFrame* iframe);

    SwsContext* ctx;
    AVFrame* oframe;
    VideoCodec iparams;
    VideoCodec oparams;
};


} // namespace ff
} // namespace base


#endif
#endif // BASE_AV_VideoConverter_H


/// @\}
