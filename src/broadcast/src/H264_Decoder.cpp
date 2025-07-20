/**
 * @file
 * HW-Accelerated decoding example.
 *
 * @example hw_decode.c
 * This example shows how to do HW-accelerated decoding with output
 * frames from the HW video surfaces.
 */

#include "H264_Decoder.h"
#include "tools.h"
#include <thread>
#include "base/logger.h"
#include "Settings.h"

using namespace base;

static enum AVPixelFormat get_qsv_format(AVCodecContext *avctx, const enum AVPixelFormat *pix_fmts)
{
    while (*pix_fmts != AV_PIX_FMT_NONE)
    {
        if (*pix_fmts == AV_PIX_FMT_QSV)
        {
            base::web_rtc::DecodeContext *decode = (base::web_rtc::DecodeContext *) avctx->opaque;
            AVHWFramesContext *frames_ctx;
            AVQSVFramesContext *frames_hwctx;
            int ret;

            /* create a pool of surfaces to be used by the decoder */
            avctx->hw_frames_ctx = av_hwframe_ctx_alloc(decode->hw_device_ref);
            if (!avctx->hw_frames_ctx) return AV_PIX_FMT_NONE;
            frames_ctx = (AVHWFramesContext *) avctx->hw_frames_ctx->data;
            frames_hwctx = (AVQSVFramesContext *) frames_ctx->hwctx;

            frames_ctx->format = AV_PIX_FMT_QSV;
            frames_ctx->sw_format = avctx->sw_pix_fmt;
            frames_ctx->width = avctx->coded_width;  // FFALIGN(avctx->coded_width,  32);
            frames_ctx->height = avctx->coded_height;  // FFALIGN(avctx->coded_height, 32);
            frames_ctx->initial_pool_size = 32;

            frames_hwctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

            ret = av_hwframe_ctx_init(avctx->hw_frames_ctx);
            if (ret < 0) return AV_PIX_FMT_NONE;

            return AV_PIX_FMT_QSV;
        }

        pix_fmts++;
    }

    fprintf(stderr, "The QSV pixel format not offered in get_format()\n");

    return AV_PIX_FMT_NONE;
}

static enum AVPixelFormat get_cuda_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != AV_PIX_FMT_NONE; p++)
    {
        if (*p == AV_PIX_FMT_CUDA)  // AV_PIX_FMT_VAAPI) // AV_PIX_FMT_QSV
            return *p;
    }

    fprintf(stderr, "Unable to decode this file using VA-API.\n");
    return AV_PIX_FMT_NONE;
}


static enum AVPixelFormat get_vaapi_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != AV_PIX_FMT_NONE; p++)
    {
        if (*p == AV_PIX_FMT_VAAPI) return *p;
    }


    fprintf(stderr, "Unable to decode this file using VA-API.\n");
    return AV_PIX_FMT_NONE;
}

#if 0



static int test_device(enum AVHWDeviceType type, const char *name,
                       const char *device, AVDictionary *opts, int flags)
{
    AVBufferRef *ref;
    AVHWDeviceContext *dev;
    int err;

    err = av_hwdevice_ctx_create(&ref, type, device, opts, flags);
    if (err < 0) {
        fprintf(stderr, "Failed to create %s device: %d.\n", name, err);
        return 1;
    }

    dev = (AVHWDeviceContext*)ref->data;
    if (dev->type != type) {
        fprintf(stderr, "Device created as type %d has type %d.\n",
                type, dev->type);
        av_buffer_unref(&ref);
        return -1;
    }

    fprintf(stderr, "Device type %s successfully created.\n", name);

    //err = test_derivation(ref, name);

    av_buffer_unref(&ref);

    return err;
}

static int test_device_type(enum AVHWDeviceType type)
{
    enum AVHWDeviceType check;
    const char *name;
    int found, err;

    name = av_hwdevice_get_type_name(type);
    if (!name) {
        fprintf(stderr, "No name available for device type %d.\n", type);
        return -1;
    }

    check = av_hwdevice_find_type_by_name(name);
    if (check != type) {
        fprintf(stderr, "Type %d maps to name %s maps to type %d.\n",
               type, name, check);
        return -1;
    }

    found = 0;

    err = test_device(type, name, NULL, NULL, 0);
    if (err < 0) {
        fprintf(stderr, "Test failed for %s with default options.\n", name);
        return -1;
    }
    if (err == 0) {
        fprintf(stderr, "Test passed for %s with default options.\n", name);
        ++found;
    }

    /*/
    for (i = 0; i < FF_ARRAY_ELEMS(test_devices); i++) {
        if (test_devices[i].type != type)
            continue;

        for (j = 0; test_devices[i].possible_devices[j]; j++) {
            err = test_device(type, name,
                              test_devices[i].possible_devices[j],
                              NULL, 0);
            if (err < 0) {
                fprintf(stderr, "Test failed for %s with device %s.\n",
                       name, test_devices[i].possible_devices[j]);
                return -1;
            }
            if (err == 0) {
                fprintf(stderr, "Test passed for %s with device %s.\n",
                        name, test_devices[i].possible_devices[j]);
                ++found;
            }
        }
    }*/

    return !found;
}
#endif


namespace base
{
namespace web_rtc
{


int H264_Decoder::hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type)
{
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&decode->hw_device_ref, type, NULL, NULL, 0)) < 0)
    {
        fprintf(stderr, "Failed to create specified HW device.\n");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(decode->hw_device_ref);

    return err;
}


int H264_Decoder::decode_write(AVCodecContext *avctx, AVPacket *packet)
{
    AVFrame *frame = NULL, *sw_frame = NULL;
    AVFrame *tmp_frame = NULL;
    uint8_t *buffer = NULL;
    int size;
    int ret = 0;

    ret = avcodec_send_packet(avctx, packet);
    if (ret < 0)
    {
        fprintf(stderr, "Error during decoding\n");
        return ret;
    }

    while (1)
    {
        if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc()))
        {
            fprintf(stderr, "Can not alloc frame\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        ret = avcodec_receive_frame(avctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            return 0;
        }
        else if (ret < 0)
        {
            fprintf(stderr, "Error while decoding\n");
            goto fail;
        }

        if (frame->format == hw_pix_fmt)
        {
            /* retrieve data from GPU to CPU */
            if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0)
            {
                fprintf(stderr, "Error transferring the data to system memory\n");
                goto fail;
            }
            tmp_frame = sw_frame;
        }
        else
            tmp_frame = frame;

        size = av_image_get_buffer_size(
            (AVPixelFormat) tmp_frame->format, tmp_frame->width, tmp_frame->height, 1);
        buffer = (uint8_t *) av_malloc(size);
        if (!buffer)
        {
            fprintf(stderr, "Can not alloc buffer\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        ret = av_image_copy_to_buffer(
            buffer,
            size,
            (const uint8_t *const *) tmp_frame->data,
            (const int *) tmp_frame->linesize,
            (AVPixelFormat) tmp_frame->format,
            tmp_frame->width,
            tmp_frame->height,
            1);
        if (ret < 0)
        {
            fprintf(stderr, "Can not copy image to buffer\n");
            goto fail;
        }

        //        if ((ret = fwrite(buffer, 1, size, output_file)) < 0) {
        //            fprintf(stderr, "Failed to dump raw data.\n");
        //            goto fail;
        //        }

    fail:
        av_frame_free(&frame);
        av_frame_free(&sw_frame);
        av_freep(&buffer);
        if (ret < 0) return ret;
    }
}


int H264_Decoder::transcode_write(AVCodecContext *avctx, AVPacket *packet)
{
    AVFrame *frame = NULL;
    // AVFrame *tmp_frame = NULL;
    // uint8_t *buffer = NULL;
    int size;
    int ret = 0;

    ret = avcodec_send_packet(avctx, packet);
    if (ret < 0)
    {
        fprintf(stderr, "Error during decoding\n");
        return ret;
    }

    while (1)
    {
        if (!(frame = av_frame_alloc()))
        {
            fprintf(stderr, "Can not alloc frame\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        ret = avcodec_receive_frame(avctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_frame_free(&frame);
            return 0;
        }
        else if (ret < 0)
        {
            fprintf(stderr, "Error while decoding\n");
            goto fail;
        }


        if (frame->pict_type == AV_PICTURE_TYPE_I)
        {
            if (trackInfo.scale == -1)
            {
                // float fr  = (cdc_ctx->time_base.den )/(cdc_ctx->time_base.num*cdc_ctx->ticks_per_frame); //
                // default
                // SInfo << "frame I";
                std::list<AVFrame *> list;
                frameQueue.push(list);
            }
             vdelay = uint64_t(1000000 * avctx->time_base.num * avctx->ticks_per_frame)
                  / uint64_t(avctx->time_base.den);  // default
            //  ++queueFilled
            //
            //vdelay = uint64_t(1000000) / uint64_t(fps);
        }


        if (trackInfo.scale == -1)
        {
            pushQueue(frame);

            return 0;
        }

        frame->pts = vframecount;

        if (frame->format == hw_pix_fmt)
        {
            /* retrieve data from GPU to CPU */
            //            if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
            //                fprintf(stderr, "Error transferring the data to system memory\n");
            //                goto fail;
            //            }
            //            tmp_frame = sw_frame;
            cb_frame(avctx, frame);
        }
        else
        {
            cb_frame(avctx, frame);
        }

        delayFrame();

        //   tmp_frame = frame;

        //        size = av_image_get_buffer_size((AVPixelFormat)tmp_frame->format, tmp_frame->width,
        //                                        tmp_frame->height, 1);
        //        buffer = (uint8_t*)av_malloc(size);
        //        if (!buffer) {
        //            fprintf(stderr, "Can not alloc buffer\n");
        //            ret = AVERROR(ENOMEM);
        //            goto fail;
        //        }
        //        ret = av_image_copy_to_buffer(buffer, size,
        //                                      (const uint8_t * const *)tmp_frame->data,
        //                                      (const int *)tmp_frame->linesize,
        //                                      (AVPixelFormat)tmp_frame->format, tmp_frame->width,
        //                                      tmp_frame->height, 1);
        //        if (ret < 0) {
        //            fprintf(stderr, "Can not copy image to buffer\n");
        //            goto fail;
        //        }
        //
        //        if ((ret = fwrite(buffer, 1, size, output_file)) < 0) {
        //            fprintf(stderr, "Failed to dump raw data.\n");
        //            goto fail;
        //        }

    fail:
        // av_frame_free(&frame);
        // av_freep(&buffer);
        if (ret < 0) return ret;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


H264_Decoder::H264_Decoder(st_track &trackInfo, AVCodecID &codec_id)
    : codec(NULL),
      codec_context(NULL),
      //      fp(NULL),
      frame(0),
      trackInfo(trackInfo)
{
    // avcodec_register_all();

    if (trackInfo.encType && trackInfo.encType != EN_VP9)
    {
        enum AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
        int pass, fail, skip, err;


        if (trackInfo.encType == EN_QUICKSYNC)
        {
            if (!Settings::configuration.haswell)
                avcodec_hw_type = AV_HWDEVICE_TYPE_QSV;
            else
                avcodec_hw_type = AV_HWDEVICE_TYPE_VAAPI;
        }
        else
            avcodec_hw_type = AV_HWDEVICE_TYPE_CUDA;

        //        pass = fail = skip = 0;
        //        while (1) {
        //            type = av_hwdevice_iterate_types(type);
        //            if (type == AV_HWDEVICE_TYPE_NONE)
        //                break;
        //
        //            err = test_device_type(type);
        //            if (err == 0) {
        //                ++pass;
        //                avcodec_hw_type = type;
        //                if( avcodec_hw_type == AV_HWDEVICE_TYPE_CUDA)
        //                    break;
        //
        //            } else if (err < 0)
        //                ++fail;
        //            else
        //                ++skip;
        //
        //
        //        }
    }

    //    if (pass)
    //    {
    //        ret = av_hwdevice_ctx_create(&avcodec_hw_device_ctx, avcodec_hw_type,
    //                                     NULL, NULL, 0);
    //
    //        if (ret < 0) {
    //          printf("avcodec: Failed to create HW device \n");
    //        }
    //
    //        if (avcodec_hw_type == AV_HWDEVICE_TYPE_CUDA) {
    //
    //           avcodec_hw_pix_fmt = AV_PIX_FMT_CUDA;
    //        } else if (avcodec_hw_type == AV_HWDEVICE_TYPE_D3D11VA) {
    //
    //           avcodec_hw_pix_fmt = AV_PIX_FMT_D3D11;
    //         avcodec_sw_pix_fmt = AV_PIX_FMT_NV12;
    //         //AV_PIX_FMT_0RGB32;   // if enabled change the #if(1) to #if(0) beneath, do not fill the the
    //         color
    //         //AV_PIX_FMT_NV12;  //
    //        // AV_PIX_FMT_YUV420P;
    //          //AV_PIX_FMT_NV12;
    //        }
    //
    //
    //
    //    }


    load(codec_id);
}

H264_Decoder::~H264_Decoder()
{
    delete decode;
    decode = nullptr;

    if (codec_context)
    {
        avcodec_close(codec_context);
        avcodec_free_context(&codec_context);
        av_free(codec_context);
        codec_context = NULL;
    }


    cb_frame = NULL;

    // frame = 0;
    //  frame_timeout = 0;

    // if (output_file)
    //     fclose(output_file);

    av_buffer_unref(&decode->hw_device_ref);


    while (frameQueue.size())
    {
        std::list<AVFrame *> &list = frameQueue.front();

        // SInfo << list.size();

        AVFrame *frame = list.front();


        av_frame_free(&frame);

        frame = nullptr;

        list.pop_front();

        if (!list.size()) frameQueue.pop();
    }
}

bool H264_Decoder::load(AVCodecID &codec_id)
{
    decode = new base::web_rtc::DecodeContext();

    // ffmpeg -decoders

    /*
        VFS..D h264                 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10
        V....D libopenh264          OpenH264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (codec h264)
        V..... h264_cuvid           Nvidia CUVID H264 decoder (codec h264)
     */
    //  codec = avcodec_find_decoder_by_name("h264_cuvid");
    //
    //  if(!codec)
    //     codec = avcodec_find_decoder_by_name("libopenh264");
    //
    //  if(!codec)
    //    codec = avcodec_find_decoder_by_name("h264");

    if (avcodec_hw_type == AV_HWDEVICE_TYPE_QSV) { codec = avcodec_find_decoder_by_name("h264_qsv"); }

    if (!codec) codec = avcodec_find_decoder(codec_id);


    if (!codec)
    {
        printf("Error: cannot find the h264 code\n");
        return false;
    }

    codec_context = avcodec_alloc_context3(codec);

    if (codec->capabilities & AV_CODEC_CAP_TRUNCATED) { codec_context->flags |= AV_CODEC_CAP_TRUNCATED; }

    codec_context->opaque = decode;

    if (trackInfo.encType && trackInfo.encType != EN_VP9)
    {
        for (int i = 0;; i++)
        {
            const AVCodecHWConfig *config = avcodec_get_hw_config(codec, i);
            if (!config)
            {
                fprintf(
                    stderr,
                    "Decoder %s does not support device type %s.\n",
                    codec->name,
                    av_hwdevice_get_type_name(avcodec_hw_type));
                return -1;
            }
            if (config->methods && config->device_type == avcodec_hw_type)
            {
                hw_pix_fmt = config->pix_fmt;
                break;
            }
        }


        if (avcodec_hw_type == AV_HWDEVICE_TYPE_D3D11VA)
        {
            //  frames_ctx->initial_pool_size = 1;
            // AVD3D11VAFramesContext* hwctx1 = (AVD3D11VAFramesContext*) frames_ctx->hwctx;  // #include
            // <libavutil/hwcontext_d3d11va.h>
            //  hwct1x->MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
            // According to hwcontex_d3d11va.h, yuv420p means
            // DXGI_FORMAT_420_OPAQUE, which has no shader support.
            // if (frames_ctx->sw_format != AV_PIX_FMT_YUV420P)
            //   hwct1x->BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }
        else if (avcodec_hw_type == AV_HWDEVICE_TYPE_CUDA)
        {
            codec_context->extra_hw_frames = 20;
            codec_context->get_format = get_cuda_format;
        }
        else if (avcodec_hw_type == AV_HWDEVICE_TYPE_VAAPI)
        {
            codec_context->extra_hw_frames = 95;
            codec_context->get_format = get_vaapi_format;
        }
        else if (avcodec_hw_type == AV_HWDEVICE_TYPE_QSV)
        {  //  codec_context->extra_hw_frames  = 95;
            codec_context->get_format = get_qsv_format;
        }


        if (hw_decoder_init(codec_context, avcodec_hw_type) < 0)
        {
            printf("failed hw_decoder_init\n");
            return -1;
        }
    }


    if (avcodec_open2(codec_context, codec, NULL) < 0)
    {
        printf("Error: could not open codec.\n");
        return false;
    }

    //    fp = fopen(filepath.c_str(), "rb");
    //
    //    if (!fp) {
    //        printf("Error: cannot open: %s\n", filepath.c_str());
    //        return false;
    //    }

    //    picture = av_frame_alloc();
    //    parser = av_parser_init(AV_CODEC_ID_H264);
    //
    //    if (!parser) {
    //        printf("Erorr: cannot create H264 parser.\n");
    //        return false;
    //    }


    // kickoff reading...
    // readBuffer();

    resetTimer();
    return true;
}

void H264_Decoder::resetTimer()
{
    startStreaming = CurrentTime_microseconds();
    vframecount = 0;
}


void H264_Decoder::decodeFrame(uint8_t *data, int size)
{
    AVPacket pkt;
    // int got_picture = 0;
    // int len = 0;

    av_init_packet(&pkt);

    pkt.data = data;
    pkt.size = size;

    //  len = avcodec_decode_video2(codec_context, picture, &got_picture, &pkt);
    //  if(len < 0) {
    //    printf("Error while decoding a frame.\n");
    //  }
    //
    //  if(got_picture == 0) {
    //    return;
    //  }
    //
    //  ++frame;
    //
    //  if(cb_frame) {
    //    cb_frame(picture, cb_user);
    //  }

    transcode_write(codec_context, &pkt);
}


void H264_Decoder::update(uint8_t *indata, int sz, AVPictureType pict_type)
{
    if (trackInfo.speed <= 3 || (trackInfo.speed > 3 && pict_type == AV_PICTURE_TYPE_I)) decodeFrame(indata, sz);
}


void H264_Decoder::delayFrame()
{

    int speed = 1;
    if (trackInfo.speed != 0) { speed = trackInfo.speed; }


    if (speed > 1 && speed < 4) { vdelay = vdelay / speed; }
    else if (speed < -1)
    {
        vdelay = vdelay * (-1 * speed);
    }


    ++vframecount;


    uint64_t deltamicro = CurrentTime_microseconds() - startStreaming;

    //    if(  deltamicro >=  vframecount * vdelay)
    //    {
    //         std::this_thread::sleep_for(std::chrono::microseconds(1004));
    //        SError <<  "deltamicro > vframecount * vdelay " << deltamicro - vframecount * vdelay;
    //    }
    //#ifdef LOCALTEST
    while (deltamicro < vframecount * vdelay)
    {
        //        SInfo <<  "deltamicro > vframecount * vdelay " << deltamicro - vframecount * vdelay;

        std::this_thread::sleep_for(std::chrono::microseconds(20000));
        deltamicro = CurrentTime_microseconds() - startStreaming;
    }
    //#endif
}


void H264_Decoder::pushQueue(AVFrame *frame)
{
    if (frame->format == AV_PIX_FMT_CUDA || frame->format == AV_PIX_FMT_QSV)
    {
        AVFrame *sw_frame = NULL;

        if (!(sw_frame = av_frame_alloc()))
        {
            fprintf(stderr, "Can not alloc frame\n");
            exit(0);
        }

        // retrieve data from GPU to CPU
        if ((av_hwframe_transfer_data(sw_frame, frame, 0)) < 0)
        {
            fprintf(stderr, "Error transferring the data to system memory\n");
            exit(0);
        }
        if (frameQueue.size() > 0) frameQueue.back().push_front(sw_frame);

        av_frame_free(&frame);
    }
    else if (frameQueue.size() > 0)
        frameQueue.back().push_front(frame);

    popQueue();
}

void H264_Decoder::popQueue()
{
    if (frameQueue.size() > 1)
    {
        std::list<AVFrame *> &list = frameQueue.front();

        // SInfo << list.size();

        AVFrame *frame = list.front();

        // int width = frame->width;
        // int height =  frame->height;

        list.pop_front();

        if (!list.size()) frameQueue.pop();


        if (frame->format == hw_pix_fmt)
        {
            /* retrieve data from GPU to CPU */
            //            if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
            //                fprintf(stderr, "Error transferring the data to system memory\n");
            //                goto fail;
            //            }
            //            tmp_frame = sw_frame;
            cb_frame(codec_context, frame);
        }
        else
        {
            cb_frame(codec_context, frame);
        }


        // av_frame_free(&frame);

        delayFrame();
    }
}
}  // namespace web_rtc
}  // namespace base
