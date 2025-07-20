#include "H264_Encoder.h"


#include "rtc_base/ref_counted_object.h"
#include "rtc_base/atomic_ops.h"
#include <chrono>
#include "base/platform.h"
#include "tools.h"
#include "common_video/h264/sps_parser.h"
#include "common_video/h264/h264_common.h"

#include "Settings.h"
#include "base/logger.h"

using namespace base;

//#define _DUMPFILE1 1

namespace base
{
namespace web_rtc
{

int H264_Encoder::set_hwframe_ctx(
    AVCodecContext *ctx, AVBufferRef *device_ctx, AVPixelFormat sw_pix_fmt, int width, int height)
{
    AVBufferRef *hw_frames_ref;
    AVHWFramesContext *frames_ctx = NULL;
    int err = 0;

    printf("avcodec: encode: create hardware frames.. (%d x %d)\n", width, height);

    if (!(hw_frames_ref = av_hwframe_ctx_alloc(device_ctx)))
    {
        printf(
            "avcodec: encode: Failed to create hardware"
            " frame context.\n");
        return ENOMEM;
    }


    ctx->hw_device_ctx = av_buffer_ref(device_ctx);
    frames_ctx = (AVHWFramesContext *) (void *) hw_frames_ref->data;
    frames_ctx->format = avcodec_hw_pix_fmt;

    ctx->pix_fmt = avcodec_hw_pix_fmt;

    frames_ctx->sw_format = sw_pix_fmt;
    frames_ctx->width = width;
    frames_ctx->height = height;
    frames_ctx->initial_pool_size = 20;  // for nvidia

    if (avcodec_hw_type == AV_HWDEVICE_TYPE_D3D11VA)
    {
        frames_ctx->initial_pool_size = 1;
        // AVD3D11VAFramesContext* hwctx1 = (AVD3D11VAFramesContext*) frames_ctx->hwctx;  // #include
        // <libavutil/hwcontext_d3d11va.h>
        //  hwct1x->MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
        // According to hwcontex_d3d11va.h, yuv420p means
        // DXGI_FORMAT_420_OPAQUE, which has no shader support.
        // if (frames_ctx->sw_format != AV_PIX_FMT_YUV420P)
        //   hwct1x->BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }


    if ((err = av_hwframe_ctx_init(hw_frames_ref)) < 0)
    {
        printf(
            "avcodec: encode:"
            " Failed to initialize hardware frame context."
            "Error code: \n");
        av_buffer_unref(&hw_frames_ref);
        return err;
    }

    ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
    if (!ctx->hw_frames_ctx) err = AVERROR(ENOMEM);

    av_buffer_unref(&hw_frames_ref);

    return err;
}


int H264_Encoder::init_filter(
    FilteringContext *fctx, AVCodecContext *dec_ctx, AVCodecContext *enc_ctx, const char *filter_spec)
{
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc = NULL;
    const AVFilter *buffersink = NULL;
    AVFilterContext *buffersrc_ctx = NULL;
    AVFilterContext *buffersink_ctx = NULL;
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVFilterGraph *filter_graph = avfilter_graph_alloc();

    if (!outputs || !inputs || !filter_graph)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        buffersrc = avfilter_get_by_name("buffer");
        buffersink = avfilter_get_by_name("buffersink");
        if (!buffersrc || !buffersink)
        {
            av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        snprintf(
            args,
            sizeof(args),
            "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            dec_ctx->width,
            dec_ctx->height,
            dec_ctx->pix_fmt,
            dec_ctx->time_base.num,
            dec_ctx->time_base.den,
            dec_ctx->sample_aspect_ratio.num,
            dec_ctx->sample_aspect_ratio.den);

        ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, filter_graph);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
            goto end;
        }

        if (encType)
        {
            AVBufferSrcParameters *par = av_buffersrc_parameters_alloc();

            if (!par) return AVERROR(ENOMEM);
            memset(par, 0, sizeof(*par));
            par->format = AV_PIX_FMT_NONE;


            par->hw_frames_ctx = dec_ctx->hw_frames_ctx;  //  ifilter->hw_frames_ctx;
            ret = av_buffersrc_parameters_set(buffersrc_ctx, par);
            if (ret < 0) goto end;
            av_freep(&par);
        }


        ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, NULL, filter_graph);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
            goto end;
        }

        ret = av_opt_set_bin(
            buffersink_ctx,
            "pix_fmts",
            (uint8_t *) &enc_ctx->pix_fmt,
            sizeof(enc_ctx->pix_fmt),
            AV_OPT_SEARCH_CHILDREN);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
            goto end;
        }
    }
    else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        buffersrc = avfilter_get_by_name("abuffer");
        buffersink = avfilter_get_by_name("abuffersink");
        if (!buffersrc || !buffersink)
        {
            av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        if (!dec_ctx->channel_layout)
            dec_ctx->channel_layout = av_get_default_channel_layout(dec_ctx->channels);
        snprintf(
            args,
            sizeof(args),
            "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
            dec_ctx->time_base.num,
            dec_ctx->time_base.den,
            dec_ctx->sample_rate,
            av_get_sample_fmt_name(dec_ctx->sample_fmt),
            dec_ctx->channel_layout);
        ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, filter_graph);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
            goto end;
        }

        ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, NULL, filter_graph);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer sink\n");
            goto end;
        }

        ret = av_opt_set_bin(
            buffersink_ctx,
            "sample_fmts",
            (uint8_t *) &enc_ctx->sample_fmt,
            sizeof(enc_ctx->sample_fmt),
            AV_OPT_SEARCH_CHILDREN);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
            goto end;
        }

        ret = av_opt_set_bin(
            buffersink_ctx,
            "channel_layouts",
            (uint8_t *) &enc_ctx->channel_layout,
            sizeof(enc_ctx->channel_layout),
            AV_OPT_SEARCH_CHILDREN);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
            goto end;
        }

        ret = av_opt_set_bin(
            buffersink_ctx,
            "sample_rates",
            (uint8_t *) &enc_ctx->sample_rate,
            sizeof(enc_ctx->sample_rate),
            AV_OPT_SEARCH_CHILDREN);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
            goto end;
        }
    }
    else
    {
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    /* Endpoints for the filter graph. */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if (!outputs->name || !inputs->name)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_spec, &inputs, &outputs, NULL)) < 0) goto end;


    if (encType)
        for (int i = 0; i < filter_graph->nb_filters; i++)
        {
            filter_graph->filters[i]->hw_device_ctx
                = av_buffer_ref(avcodec_hw_device_ctx);  // av_buffer_ref(enc_ctx->hw_device_ctx );
        }


    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot setavfilter_graph_config\n");
        goto end;
    }

    /* Fill FilteringContext */
    fctx->buffersrc_ctx = buffersrc_ctx;
    fctx->buffersink_ctx = buffersink_ctx;
    fctx->filter_graph = filter_graph;

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}


int H264_Encoder::init_filters(AVCodecContext *dec_ctx)
{
    unsigned int i;
    int ret;

    char filter_spec[512] = {'n', 'u', 'l', 'l', '\0'};

    filter_ctx = (FilteringContext *) av_malloc_array(1, sizeof(*filter_ctx));
    if (!filter_ctx) return AVERROR(ENOMEM);

    for (i = 0; i < 1; i++)
    {
        filter_ctx[i].buffersrc_ctx = NULL;
        filter_ctx[i].buffersink_ctx = NULL;
        filter_ctx[i].filter_graph = NULL;

        //  if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if (!encType)
                snprintf(
                    filter_spec,
                    sizeof(filter_spec),
                    "scale=%d:%d",
                    c->width,
                    c->height);  // filter_spec = "scale=w=iw/2:-1";
            else if (encType == EN_NVIDIA)
            {
                // filter_spec =  "scale_cuda=w=iw/2:-1"; //scale_cuda=640:480"
                snprintf(filter_spec, sizeof(filter_spec), "scale_cuda=%d:%d", c->width, c->height);
            }
            else if (encType == EN_QUICKSYNC)
            {
                // filter_spec =  "scale_cuda=w=iw/2:-1"; //scale_cuda=640:480"
                snprintf(filter_spec, sizeof(filter_spec), "scale_vaapi=%d:%d", c->width, c->height);
            }
            // filter_spec = "drawtext=fontfile=FreeSerif.ttf: text='%{localtime}': x=w-text_w: y=0:
            // fontsize=24: fontcolor=yellow@1.0: box=1: boxcolor=red@1.0"; filter_spec =
            // "drawtext=fontfile=FreeSerif.ttf :text='test': x=w-text_w: y=text_h: fontsize=24:
            // fontcolor=yellow@1.0: box=1: boxcolor=red@1.0";


            // filter_spec = "rate=6"; //filter_spec = "null"; /* passthrough (dummy) filter for video */
        }


        ret = init_filter(&filter_ctx[i], dec_ctx, c, filter_spec);
        if (ret) return ret;

        filter_ctx[i].enc_pkt = av_packet_alloc();
        if (!filter_ctx[i].enc_pkt) return AVERROR(ENOMEM);

        filter_ctx[i].filtered_frame = av_frame_alloc();
        if (!filter_ctx[i].filtered_frame) return AVERROR(ENOMEM);
    }
    return 0;
}


H264_Encoder::H264_Encoder(en_EncType &encType)
    : codec(NULL),
      c(NULL)
      //  ,fp(NULL)
      ,
      sw_frame(nullptr),
      encType(encType)
{
    // avcodec_register_all();


    if (encType == EN_NVIDIA)
    {
        codec = avcodec_find_encoder_by_name("h264_nvenc");
        avcodec_hw_type = AV_HWDEVICE_TYPE_CUDA;
    }


    if (encType == EN_QUICKSYNC)
    {
        if (Settings::configuration.haswell)
        {
            codec = avcodec_find_encoder_by_name("h264_vaapi");
            avcodec_hw_type = AV_HWDEVICE_TYPE_VAAPI;
        }
        else
        {
            codec = avcodec_find_encoder_by_name("h264_qsv");
            avcodec_hw_type = AV_HWDEVICE_TYPE_QSV;
        }
    }

    if (!codec)
    {
        // fprintf(stderr, "Codec not found\n");
        //  exit(1);

        codec = avcodec_find_encoder_by_name("libx264");
        if (!codec)
        {
            fprintf(stderr, "Codec not found\n");
            exit(1);
        }
    }

    c = avcodec_alloc_context3(codec);
    if (!c)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

#if _DUMPFILE1

    std::string filename = "/tmp/test.264";

    fp = fopen(filename.c_str(), "wb");
    if (!fp)
    {
        fprintf(stderr, "Could not open %s\n", filename.c_str());
        exit(1);
    }

    printf(" opened %s\n", filename.c_str());

#endif


    pkt = av_packet_alloc();
    if (!pkt)
    {
        fprintf(stderr, "could not allocate the packet\n");
        exit(1);
    }


    if (!encType)  // software
        return;


    //    enum AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
    //    int pass, fail, skip, err;
    //
    //    pass = fail = skip = 0;
    //    while (1) {
    //        type = av_hwdevice_iterate_types(type);
    //        if (type == AV_HWDEVICE_TYPE_NONE)
    //            break;
    //
    //        err = test_device_type(type);
    //        if (err == 0) {
    //            ++pass;
    //            avcodec_hw_type = type;
    //          //  break;
    //        } else if (err < 0)
    //            ++fail;
    //        else
    //            ++skip;
    //
    //
    //    }
    //
    //    if (pass)
    {
        ret = av_hwdevice_ctx_create(&avcodec_hw_device_ctx, avcodec_hw_type, NULL, NULL, 0);

        if (ret < 0) { printf("avcodec: Failed to create HW device \n"); }

        if (avcodec_hw_type == AV_HWDEVICE_TYPE_CUDA) { avcodec_hw_pix_fmt = AV_PIX_FMT_CUDA; }
        else if (avcodec_hw_type == AV_HWDEVICE_TYPE_D3D11VA)
        {
            avcodec_hw_pix_fmt = AV_PIX_FMT_D3D11;
            avcodec_sw_pix_fmt = AV_PIX_FMT_NV12;
            // AV_PIX_FMT_0RGB32;   // if enabled change the #if(1) to #if(0) beneath, do not fill the the color
            // AV_PIX_FMT_NV12;  //
            // AV_PIX_FMT_YUV420P;
            // AV_PIX_FMT_NV12;
        }
        else if (avcodec_hw_type == AV_HWDEVICE_TYPE_QSV)
        {
            avcodec_hw_pix_fmt = AV_PIX_FMT_QSV;
            // avcodec_sw_pix_fmt = AV_PIX_FMT_NV12;
        }
        else if (avcodec_hw_type == AV_HWDEVICE_TYPE_VAAPI)
        {
            avcodec_hw_pix_fmt = AV_PIX_FMT_VAAPI;
        }

        if (!(hw_frame = av_frame_alloc()))
        {
            fprintf(stderr, "Hw frame is not allocated");
            exit(0);
        }
    }
}

H264_Encoder::~H264_Encoder()
{
    clean();

    if (pkt)
    {
        av_packet_free(&pkt);

        pkt = NULL;
    }

    if (c)
    {
        // avcodec_close(c);
        // av_free(c);
        avcodec_free_context(&c);
        c = NULL;
    }

    //  if(sw_frame) {
    //    av_frame_free(&sw_frame);
    //    sw_frame = NULL;
    //  }


    if (hw_frame)
    {
        av_frame_free(&hw_frame);
        hw_frame = NULL;
    }


#if _DUMPFILE1
    if (fp)
    {
        fclose(fp);
        fp = NULL;
    }
#endif


    if (avcodec_hw_device_ctx) av_buffer_unref(&avcodec_hw_device_ctx);
}


bool H264_Encoder::load(int fps, int width, int height)
{
    //  const AVCodec* audiocodec = NULL;
    //  audiocodec = avcodec_find_encoder_by_name("libfdk_aac");  // Specify the use of file encoding type


    // avcodec_get_context_defaults3(c, codec);

    c->pix_fmt = AV_PIX_FMT_YUV420P;
    // c->time_base.num         = 1;
    // c->time_base.den         = 25;
    c->profile = FF_PROFILE_H264_HIGH;
    c->level = 31;


    /* put sample parameters */
    //  c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = width;
    c->height = height;
    /* frames per second */

    AVRational tb;
    tb.num = 1;
    tb.den = fps;
    c->time_base = tb;  // arvind

    AVRational tfp;
    tfp.num = fps;
    tfp.den = 1;

    c->framerate = tfp;

    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    // c->gop_size = fps;
    //  c->max_b_frames = 1;
    //   c->max_b_frames = 3;
    //  c->pix_fmt = avcodec_sw_pix_fmt;

    // c->color_range = AVCOL_RANGE_JPEG;


    //  c->bit_rate = config.target_bps * 0.7;
    //   c->rc_max_rate = config.target_bps * 0.85;
    //  c->rc_min_rate = config.target_bps * 0.1;
    //  c->rc_buffer_size = config.target_bps * 2;


    if (codec->id == AV_CODEC_ID_H264)
    {
        // av_opt_set(c->priv_data, "preset", "slow", 0);
        //  av_opt_set(c->priv_data, "preset", "ultrafast", 0);
        // av_opt_set(c->priv_data, "tune", "zerolatency", 0);
    }


    if (avcodec_hw_type != AV_HWDEVICE_TYPE_NONE)
    {
        av_opt_set(c->priv_data, "tune", "zerolatency", 0);

        c->level = 31;
        av_opt_set(c->priv_data, "profile", "main", 31);

        // c->level = 40;  //basline not supported in my card
        // av_opt_set(c->priv_data, "profile", "high", 0);


        int err = set_hwframe_ctx(c, avcodec_hw_device_ctx, AV_PIX_FMT_YUV420P, width, height);
        if (err < 0)
        {
            printf(
                "avcodec: encode: Failed to set"
                " hwframe context.\n");
            return false;
        }
    }

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0)
    {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }


    sw_frame = av_frame_alloc();
    if (!sw_frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }


    if (avcodec_hw_type != AV_HWDEVICE_TYPE_NONE)
    {
        hw_frame = av_frame_alloc();
        if (!hw_frame) { return false; }


        // frame->format = c->pix_fmt;
        sw_frame->width = c->width;
        sw_frame->height = c->height;

        sw_frame->format = avcodec_sw_pix_fmt;
        av_frame_get_buffer(sw_frame, 0);


        int err;
        if ((err = av_hwframe_get_buffer(c->hw_frames_ctx, hw_frame, 0)) < 0)
        {
            printf("avcodec: encode: Error code:\n");
            return false;
        }

        if (!hw_frame->hw_frames_ctx) { return false; }
    }
    else
    {
        sw_frame->format = c->pix_fmt;
        sw_frame->width = c->width;
        sw_frame->height = c->height;

        ret = av_frame_get_buffer(sw_frame, 32);
        if (ret < 0)
        {
            fprintf(stderr, "Could not allocate the video frame data\n");
            exit(1);
        }
    }


    return true;
}


void H264_Encoder::encodeFrame(AVCodecContext *dec_ctx, AVFrame *frame, int w, int h)
{
    if (!filter_ctx)
    {
        // filter_ctx = new FilteringContext();
        // std::string fileName = "/tmp/test2.264";
        if (!dec_ctx->time_base.num)
        {
            dec_ctx->time_base.num = 1;
            dec_ctx->time_base.den = 60;

            dec_ctx->framerate.num = 30;
            dec_ctx->framerate.den = 1;
        }
        int fr = (dec_ctx->time_base.den) / (dec_ctx->time_base.num * dec_ctx->ticks_per_frame);


        // load(fileName, fr , frame->width/2,  frame->height/2);

        c->pix_fmt = AV_PIX_FMT_YUV420P;  // AV_PIX_FMT_VAAPI;


        if (encType)
        {
            /*
             if( frame->format == AV_PIX_FMT_NV12)
             {
                 if(set_hwframe_ctx(c, avcodec_hw_device_ctx,AV_PIX_FMT_NV12, w, h) < 0)
                 {
                    printf("avcodec: encode: Failed to set hwframe context.\n");
                    exit(0);
                 }



                 if (( av_hwframe_get_buffer(c->hw_frames_ctx, hw_frame, 0)) < 0) {
                    fprintf(stderr, "Hw frame is not initialized");
                     exit(0);
                 }

                 if (!hw_frame->hw_frames_ctx) {

                       fprintf(stderr, "Hw frame is not initialized");
                        exit(0);
                 }



             }
             else
            */
            {
                c->hw_frames_ctx = av_buffer_ref(dec_ctx->hw_frames_ctx);
                if (!c->hw_frames_ctx)
                {
                    fprintf(stderr, "Failed to open hw_frames_ctx \n");
                    return;
                }

                c->pix_fmt = avcodec_hw_pix_fmt;
            }
        }
        /* set AVCodecContext Parameters for encoder, here we keep them stay
         * the same as decoder.
         * xxx: now the sample can't handle resolution change case.
         */
        c->time_base = av_inv_q(dec_ctx->framerate);

        c->width = w;  // dec_ctx->width;
        c->height = h;  // dec_ctx->height;


        c->gop_size = 300;
        c->max_b_frames = 0;


        c->bit_rate = 100000;
        c->rc_max_rate = 200000;

        // c->max_b_frames = 3;

        //        c->level = 40;
        //        av_opt_set(c->priv_data, "profile", "main", 0);

        c->level = 31;

        if (std::string(c->codec->name) == "libx264")  // ffmpeg -h encoder=libx264
        {
            av_opt_set(c->priv_data, "profile", "main", 0);  // main or
            av_opt_set(c->priv_data, "preset", "medium", 0);  // fast is  baseline
            av_opt_set(c->priv_data, "tune", "zerolatency", 0);
        }
        else if (
            std::string(c->codec->name) == "h264_vaapi"
            || std::string(c->codec->name) == "h264_nvenc")  // ffmpeg -h encoder=h264_nvenc  // ffprobe -i
                                                             // test.264 -show_frames | grep 'pict_type'
        {
            av_opt_set(c->priv_data, "profile", "high", 0);  // main or

            av_opt_set(c->priv_data, "preset", "fast", 0);
            av_opt_set(c->priv_data, "tune", "ll", 0);
        }

        init_filters(dec_ctx);
        if ((ret = avcodec_open2(c, codec, NULL)) < 0)
        {
            fprintf(stderr, "Failed to open encode codec. Error code: \n");
            return;
        }
    }
    frame->pts = decoderCount++;
    flags = AV_BUFFERSRC_FLAG_KEEP_REF;
    int ret = filter_encode_write_frame(frame, 0, flags);
    if (ret < 0) { fprintf(stderr, "filter_encode_write_frame failed"); }

    return;


    //    SInfo << "About to Encode frame" << frame->pts ;

    av_init_packet(pkt);
    pkt->data = NULL;  // packet data will be allocated by the encoder
    pkt->size = 0;


    ret = avcodec_send_frame(c, frame);


    if (ret < 0)
    {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(c, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            // av_packet_unref(pkt);  // do not do it
            return;
        }
        else if (ret < 0)
        {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }


        // SInfo << "Encoded h264 Frame " <<  frameCount << " size " <<   pkt->size;

        ++frameCount;

        // fwrite(pkt->data, 1, pkt->size, fp);

#if _DUMPFILE1
        fwrite(pkt->data, 1, pkt->size, fp);
#endif

        write_frame(pkt);


        av_packet_unref(pkt);
    }
}


int H264_Encoder::encode_write_frame(unsigned int stream_index, int flush)
{
    // StreamContext *stream = &stream_ctx[stream_index];
    FilteringContext *filter = &filter_ctx[stream_index];
    AVFrame *filt_frame = flush ? NULL : filter->filtered_frame;
    AVPacket *enc_pkt = filter->enc_pkt;
    int ret;

    // av_log(NULL, AV_LOG_INFO, "Encoding frame\n");
    /* encode filtered frame */
    av_packet_unref(enc_pkt);

    ret = avcodec_send_frame(c, filt_frame);

    if (ret < 0) return ret;

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(c, enc_pkt);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return 0;

        /* prepare packet for muxing */
        enc_pkt->stream_index = stream_index;
        // av_packet_rescale_ts(enc_pkt,
        //                      c->time_base,
        //                      ofmt_ctx->streams[stream_index]->time_base);

        // av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
        ++frameCount;
        // printf("Write frame %3d (size=%5d)\n", frameCount, enc_pkt->size);

#if _DUMPFILE1
        fwrite(enc_pkt->data, 1, enc_pkt->size, fp);
#endif

        write_frame(enc_pkt);
        /* mux encoded frame */
        // ret = av_interleaved_write_frame(ofmt_ctx, enc_pkt);
    }

    return ret;
}

int H264_Encoder::filter_encode_write_frame(AVFrame *frame, unsigned int stream_index, int flags)
{
    FilteringContext *filter = &filter_ctx[stream_index];
    int ret;

    // av_log(NULL, AV_LOG_INFO, "Pushing decoded frame to filters\n");
    /* push the decoded frame into the filtergraph */
    ret = av_buffersrc_add_frame_flags(filter->buffersrc_ctx, frame, flags);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
        return ret;
    }

    /* pull filtered frames from the filtergraph */
    while (1)
    {
        // av_log(NULL, AV_LOG_INFO, "Pulling filtered frame from filters\n");
        ret = av_buffersink_get_frame(filter->buffersink_ctx, filter->filtered_frame);
        if (ret < 0)
        {
            /* if no more frames for output - returns AVERROR(EAGAIN)
             * if flushed and no more frames for output - returns AVERROR_EOF
             * rewrite retcode to 0 to show it as normal procedure completion
             */
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) ret = 0;
            break;
        }

        filter->filtered_frame->pict_type = AV_PICTURE_TYPE_NONE;
        ret = encode_write_frame(stream_index, 0);
        av_frame_unref(filter->filtered_frame);
        if (ret < 0) break;
    }

    return ret;
}


void H264_Encoder::clean()
{
    if (filter_ctx)
    {
        av_free(filter_ctx);
        filter_ctx = nullptr;
    }


    if (c)
    {
        // avcodec_close(c);
        // av_free(c);
        avcodec_free_context(&c);
        c = NULL;
    }
}

}  // namespace web_rtc
}  // namespace base
