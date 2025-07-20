/*

  H264_Encoder
  ---------------------------------------

  Example that shows how to use the libav parser system. This class forces a
  H264 parser and codec. You use it by opening a file that is encoded with x264
  using the `load()` function. You can pass the framerate you want to use for
  playback. If you don't pass the framerate, we will detect it as soon as the
  parser found the correct information.

  After calling load(), you can call readFrame() which will read a new frame
  when necessary. It will also make sure that it will read enough data from the
  buffer/file when there is not enough data in the buffer.

  `readFrame()` will trigger calls to the given `h264_decoder_callback` that you
  pass to the constructor.

 */

#ifndef H264_ENCODER_H
#define H264_ENCODER_H

#define H264_INBUF_SIZE 16384 /* number of bytes we read per chunk */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#if 0
#    include <tinylib.h>
#endif

extern "C"
{
#include <libavcodec/avcodec.h>

#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>

#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
}

#include "framefilter.h"

struct LayerConfig
{
    int simulcast_idx = 0;
    int width = -1;
    int height = -1;
    bool sending = true;
    bool key_frame_request = false;
    float max_frame_rate = 0;
    uint32_t target_bps = 0;
    uint32_t max_bps = 0;
    bool frame_dropping_on = false;
    int key_frame_interval = 0;

    void SetStreamState(bool send_stream);
};

class H264_Encoder
{
public:
    H264_Encoder(en_EncType encType, std::string filename); /* pass in a callback function that is called
                                                               whenever we decoded a video frame, make sure to
                                                               call `readFrame()` repeatedly */
    ~H264_Encoder(); /* d'tor, cleans up the allocated objects and closes the
                        codec context */

    bool load(int fps, int width, int height);

    bool reconfigure(LayerConfig &config);

    /* load a video file which is encoded with x264 */
    /// bool readFrame(); /* read a frame if necessary */

    // void encodeFrame(uint8_t* ydata, int ysize, uint8_t* udata, int usize,
    // uint8_t* vdata, int vsize);

    void encodeFrame();

    void clean();

    virtual void encodeFrame(AVCodecContext *dec_ctx, AVFrame *hw_frame, int w, int h);

    void encodeFrameCuda(AVCodecContext *dec_ctx, AVFrame *frame, int w, int h);

    void encodeFrameQsv(AVCodecContext *dec_ctx, AVFrame *frame, int w, int h);

    int init_filters(AVCodecContext *dec_ctx);

    int flags{0};

protected:
    // bool update(bool& needsMoreBytes); /* internally used to update/parse the
    // data we read from the buffer or file */ int readBuffer(); /* read a bit
    // more data from the buffer */
    // void encodeFrame(uint8_t* data, int size);
    /* decode a frame we read from the buffer */

    typedef struct FilteringContext
    {
        AVFilterContext *buffersink_ctx{nullptr};
        ;
        AVFilterContext *buffersrc_ctx{nullptr};
        ;
        AVFilterGraph *filter_graph{nullptr};
        ;

        AVPacket *enc_pkt{nullptr};
        AVFrame *filtered_frame{nullptr};
        ;
    } FilteringContext;

    FilteringContext *filter_ctx{nullptr};

    int init_filter(
        FilteringContext *fctx, AVCodecContext *dec_ctx, AVCodecContext *enc_ctx, const char *filter_spec);

    int filter_encode_write_frame(AVFrame *frame, unsigned int stream_index, int flags);

    int encode_write_frame(unsigned int stream_index, int flush);

    virtual void write_frame(AVPacket *enc_pkt){};

public:
    AVCodec *codec{nullptr}; /* the AVCodec* which represents the H264 decoder */
    AVCodecContext *c; /* the context; keeps generic state */
    /* parser that is used to decode the h264 bitstream */
    /* will contain a decoded picture */
    // int8_t inbuf[H264_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE]; /* used to
    // read chunks from the file */
    FILE *fp; /* file pointer to the file from which we read the h264 data */
    // int frame; /* the number of decoded frames */
    /// const char *filename, *codec_name;
    // const AVCodec *codec;
    // AVCodecContext *c= NULL;

    int decoderCount{0};

    int frameCount{0};
    int ret, x, y, got_output;
    // FILE *f;

    AVFrame *sw_frame{NULL};
    AVPacket *pkt{NULL};
    /* buffer we use to keep track of read/unused bitstream data */

    int set_hwframe_ctx(
        AVCodecContext *ctx, AVBufferRef *device_ctx, AVPixelFormat sw_pix_fmt, int width, int height);

    AVFrame *hw_frame{NULL};
    // hardware
    AVBufferRef *avcodec_hw_device_ctx{NULL};

    AVHWDeviceType avcodec_hw_type{AV_HWDEVICE_TYPE_QSV};

    en_EncType encType{EN_NONE};

    AVPixelFormat avcodec_hw_pix_fmt{AV_PIX_FMT_CUDA};
    AVPixelFormat avcodec_sw_pix_fmt{AV_PIX_FMT_YUV420P};

    int lastFrormat{0};
};

#endif
