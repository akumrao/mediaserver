


#ifndef H264_DECODER_H
#define H264_DECODER_H

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>

// qsv
#include "libavutil/buffer.h"
#include "libavutil/error.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_qsv.h"
#include "libavutil/mem.h"
}

#include "webrtc/rawVideoFrame.h"

#include <string>
#include <vector>

#include "framefilter.h"

#include <functional>

#include <list>
#include <queue>


#define H264_INBUF_SIZE 16384 /* number of bytes we read per chunk */


#define FF_INPUT_BUFFER_PADDING_SIZE 32

namespace base {
    namespace web_rtc {

/* the decoder callback, which will be called when we have decoded a frame */

class H264_Decoder
{
public:
    H264_Decoder(
        st_track &trackInfo, AVCodecID &codec_id); /* pass in a callback function that is called whenever we decoded a
                                              video frame, make sure to call `readFrame()` repeatedly */
    virtual ~H264_Decoder(); /* d'tor, cleans up the allocated objects and closes the codec context */


    // AVBufferRef *hw_device_ctx{ NULL};

    std::function<void(AVCodecContext *dec_ctx, AVFrame *frame)> cb_frame;


    int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type);


    int decode_write(AVCodecContext *avctx, AVPacket *packet);

    void update(uint8_t *data, int sz, AVPictureType pict_type);


    int transcode_write(AVCodecContext *avctx, AVPacket *packet);

    void init_hw();
    int int_dec();

    /* d'tor, cleans up the allocated objects and closes the codec context */
    bool load(AVCodecID &codec_id); /* load a video file which is encoded with x264 */
    // bool readFrame();                                                                      /* read a frame if
    // necessary */

private:
    //  bool update(bool& needsMoreBytes);                                                     /* internally
    //  used to update/parse the data we read from the buffer or file */
    // int readBuffer();                                                                      /* read a bit more
    // data from the buffer */
    void decodeFrame(uint8_t *data, int size); /* decode a frame we read from the buffer */

public:
    AVCodec *codec{nullptr}; /* the AVCodec* which represents the H264 decoder */
    AVCodecContext *codec_context; /* the context; keeps generic state */
    //    AVCodecParserContext* parser;                                                          /* parser that
    //    is used to decode the h264 bitstream */ AVFrame* picture; /* will contain a decoded picture */
    uint8_t inbuf[H264_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE]; /* used to read chunks from the file */
    // FILE *fp; /* file pointer to the file from which we read the h264 data */
    int frame; /* the number of decoded frames */

    // uint64_t frame_timeout;                                                                /* timeout when we
    // need to parse a new frame */
    //  uint64_t frame_delay;                                                                  /* delay between
    //  frames (in ns) */
    // std::vector<uint8_t> buffer;


    AVHWDeviceType avcodec_hw_type{AV_HWDEVICE_TYPE_NONE};

    void delayFrame();

    uint64_t vdelay{0};

    uint64_t vframecount{0};

    uint64_t startStreaming{0};

    st_track &trackInfo;

    int fps;
    int width;
    int height;

protected:
    std::queue<std::list<AVFrame *>> frameQueue;

    virtual void pushQueue(AVFrame *frame);

    virtual void popQueue();

    enum AVPixelFormat hw_pix_fmt
    {
        AV_PIX_FMT_CUDA
    };  // = AV_PIX_FMT_NONE;

public:
    base::web_rtc::DecodeContext *decode{NULL};

    void resetTimer();
};
}}
#endif
