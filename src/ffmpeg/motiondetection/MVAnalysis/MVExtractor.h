extern "C" {
#include <libavutil/motion_vector.h>
#include <libavformat/avformat.h>
}

#include "MVDetector.h"

#ifndef H264_MVE_H
#define H264_MVE_H

class MVExtractor {
public:

    MVExtractor(const char *src_filename);

    ~MVExtractor();

    AVFormatContext *fmt_ctx{NULL};
    AVCodecContext *video_dec_ctx{NULL};
    AVStream *video_stream{NULL};
    //const char *src_filename{NULL};

    int video_stream_idx{-1};
    AVFrame *frame{NULL};
    AVPacket pkt;
    int video_frame_count{0};

    int dst_width{0};
    int dst_height{0};
    void extract();

private:

    MVDetector *mv_detector{nullptr};

    int open_codec_context(AVFormatContext *fmt_ctx, enum AVMediaType type);

    int decode_packet(int *got_frame, int cached);

    int read(const char *src_filename);




};


#endif