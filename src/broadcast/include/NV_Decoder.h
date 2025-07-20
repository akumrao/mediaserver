
#include "H264_Decoder.h"

#ifndef NV_Decoder_H
#define NV_Decoder_H
namespace base {
    namespace web_rtc {

class NV_Decoder: public H264_Decoder {
public:
    NV_Decoder(st_track &trackInfo, AVCodecID &codec_id ); /* pass in a callback function that is called whenever we decoded a video frame, make sure to call `readFrame()` repeatedly */
    ~NV_Decoder(); /* d'tor, cleans up the allocated objects and closes the codec context */


    void pushQueue( AVFrame *frame);

    void popQueue( );

    
};
}
}
#endif
