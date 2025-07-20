

#include "config.h"

#include <stdio.h>

extern "C" {
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libavutil/buffer.h"
#include "libavutil/error.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_qsv.h"
#include "libavutil/mem.h"
    
    
#include <libswscale/swscale.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>

#include "libavutil/imgutils.h"	/// for open cv display  otherwise not required

}

#include "opencv2/opencv.hpp"
/// apt-get install libopencv-dev

typedef struct DecodeContext {
	AVBufferRef *hw_device_ref;
} DecodeContext;


const char *filter_descr = "vpp_qsv=w=640:h=360";
//const char *filter_descr = "[in]vpp_qsv=w=1024:h=768:format=nv12[out]";
//const char *filter_descr = "scale=78:24,transpose=cclock";

typedef struct FilteringContext {
	AVCodecContext  *dec_ctx;
	AVCodecContext  *enc_ctx;
	AVFilterContext *buffersink_ctx;
	AVFilterContext *buffersrc_ctx;
	AVFilterGraph *filter_graph;
	int initiallized;
} FilteringContext;
static FilteringContext *filter_ctx;

static int init_filters(void)
{
	unsigned int i;

	filter_ctx = (FilteringContext *)av_malloc_array(1, sizeof(*filter_ctx));
	if (!filter_ctx)
		return AVERROR(ENOMEM);

	filter_ctx[0].dec_ctx = NULL;
	filter_ctx[0].enc_ctx = NULL;
	filter_ctx[0].buffersrc_ctx = NULL;
	filter_ctx[0].buffersink_ctx = NULL;
	filter_ctx[0].filter_graph = NULL;
	filter_ctx[0].initiallized = 0;

	return 0;
}


static int init_filter(FilteringContext* fctx, AVCodecContext *dec_ctx, const char *filter_spec)
{
	char args[512];
	int ret = 0;
	AVFilter *buffersrc = NULL;
	AVFilter *buffersink = NULL;
	AVFilterContext *buffersrc_ctx = NULL;
	AVFilterContext *buffersink_ctx = NULL;
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs = avfilter_inout_alloc();
	AVFilterGraph *filter_graph = avfilter_graph_alloc();

	if (!outputs || !inputs || !filter_graph) {
		ret = AVERROR(ENOMEM);
		goto end;
	}

	if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
		buffersrc = (AVFilter *)avfilter_get_by_name("buffer");
		buffersink = (AVFilter *)avfilter_get_by_name("buffersink");
		if (!buffersrc || !buffersink) {
			av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}

		snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d"
			":frame_rate=%d/%d",
			dec_ctx->width, dec_ctx->height, AV_PIX_FMT_QSV, // dec_ctx->pix_fmt,
			dec_ctx->time_base.num, dec_ctx->time_base.den,
			dec_ctx->sample_aspect_ratio.num,
			dec_ctx->sample_aspect_ratio.den,
			dec_ctx->framerate.num, dec_ctx->framerate.den);

		ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			args, NULL, filter_graph);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
			goto end;
		}

		if (dec_ctx->hw_frames_ctx) {
			AVBufferSrcParameters *par = av_buffersrc_parameters_alloc();
			par->hw_frames_ctx = dec_ctx->hw_frames_ctx;
			ret = av_buffersrc_parameters_set(buffersrc_ctx, par);
			av_freep(&par);
			if (ret < 0)
				goto end;
		}

		ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
			NULL, NULL, filter_graph);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
			goto end;
		}
	}
	else {
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

	if (!outputs->name || !inputs->name) {
		ret = AVERROR(ENOMEM);
		goto end;
	}

	if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_spec,
		&inputs, &outputs, NULL)) < 0)
		goto end;

	if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
		goto end;

	/* Fill FilteringContext */
	fctx->buffersrc_ctx = buffersrc_ctx;
	fctx->buffersink_ctx = buffersink_ctx;
	fctx->filter_graph = filter_graph;
	fctx->initiallized = 1;

end:
	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);

	return ret;
}




AVPixelFormat get_format(AVCodecContext *avctx, const enum AVPixelFormat *pix_fmts)
{
	while (*pix_fmts != AV_PIX_FMT_NONE) {
		if (*pix_fmts == AV_PIX_FMT_QSV) {
			DecodeContext *decode = (DecodeContext *)avctx->opaque;
			AVHWFramesContext  *frames_ctx;
			AVQSVFramesContext *frames_hwctx;
			int ret;

			/* create a pool of surfaces to be used by the decoder */
			avctx->hw_frames_ctx = av_hwframe_ctx_alloc(decode->hw_device_ref);
			if (!avctx->hw_frames_ctx)
				return AV_PIX_FMT_NONE;
			frames_ctx = (AVHWFramesContext*)avctx->hw_frames_ctx->data;
			frames_hwctx = (AVQSVFramesContext *)frames_ctx->hwctx;

			frames_ctx->format = AV_PIX_FMT_QSV;
			frames_ctx->sw_format = avctx->sw_pix_fmt;
			frames_ctx->width = FFALIGN(avctx->coded_width, 32);
			frames_ctx->height = FFALIGN(avctx->coded_height, 32);
			frames_ctx->initial_pool_size = 32;

			frames_hwctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

			ret = av_hwframe_ctx_init(avctx->hw_frames_ctx);
			if (ret < 0)
				return AV_PIX_FMT_NONE;

			return AV_PIX_FMT_QSV;
		}

		pix_fmts++;
	}

	fprintf(stderr, "The QSV pixel format not offered in get_format()\n");

	return AV_PIX_FMT_NONE;
}
#if 1
static int decode_packet(AVCodecContext *decoder_ctx,
	AVFrame *frame, int *got_frame,
	AVPacket *pkt)
{
	int ret = 0;

	*got_frame = 0;

	ret = avcodec_send_packet(decoder_ctx, pkt);
	if (ret < 0) {
		fprintf(stderr, "Error during decoding\n");
		return ret;
	}

	while (ret >= 0) {
		int i, j;

		ret = avcodec_receive_frame(decoder_ctx, frame);
		//std::cout << "avcodec_receive_frame -- " << ret << std::endl;
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		else if (ret < 0) {
			fprintf(stderr, "Error during decoding\n");
			return ret;
		}

		//get one decoded frame, set got_frame to 1, return;
		*got_frame = 1;
		return 0;
	}

	return 0;
}
#else

static int decode_packet(DecodeContext *decode, AVCodecContext *decoder_ctx,
                         AVFrame *frame, AVFrame *sw_frame,
                         AVPacket *pkt, AVIOContext *output_ctx)
{
    int ret = 0;

    ret = avcodec_send_packet(decoder_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error during decoding\n");
        return ret;
    }

    while (ret >= 0) {
        int i, j;

        ret = avcodec_receive_frame(decoder_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            return ret;
        }

        /* A real program would do something useful with the decoded frame here.
         * We just retrieve the raw data and write it to a file, which is rather
         * useless but pedagogic. */
        ret = av_hwframe_transfer_data(sw_frame, frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error transferring the data to system memory\n");
            goto fail;
        }

        for (i = 0; i < FF_ARRAY_ELEMS(sw_frame->data) && sw_frame->data[i]; i++)
            for (j = 0; j < (sw_frame->height >> (i > 0)); j++)
                avio_write(output_ctx, sw_frame->data[i] + j * sw_frame->linesize[i], sw_frame->width);

fail:
        av_frame_unref(sw_frame);
        av_frame_unref(frame);

        if (ret < 0)
            return ret;
    }

    return 0;
}
#endif


static int get_filtered_frame(AVFrame *frame, AVFrame *filt_frame)
{
	int ret;

	ret = av_buffersrc_add_frame_flags(filter_ctx->buffersrc_ctx,
		frame, 0);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
		return ret;
	}

	ret = av_buffersink_get_frame(filter_ctx->buffersink_ctx,
		filt_frame);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error while getting data from the filtergraph\n");
		return ret;
	}

	return 0;
}

static int display_qsv_frame(AVFrame *frame, AVFrame *sw_frame)
{
	int ret;

	cv::Mat imgGray(frame->height, frame->width, CV_8U);
	cv::Mat imgColor(frame->height, frame->width, CV_8UC3);


#if 1
	/* A real program would do something useful with the decoded frame here.
	* We just retrieve the raw data and write it to a file, which is rather
	* useless but pedagogic. */
	ret = av_hwframe_transfer_data(sw_frame, frame, 0);
	if (ret < 0) {
		fprintf(stderr, "Error transferring the data to system memory\n");
		return ret;
	}
#endif


#if 1
	enum AVPixelFormat src_pix_fmt = AV_PIX_FMT_NV12, dst_pix_fmt = AV_PIX_FMT_BGR24;
	AVFrame *pFrameRGB = av_frame_alloc();
	uint8_t *out_buffer = new uint8_t[av_image_get_buffer_size(dst_pix_fmt, sw_frame->width, sw_frame->height, 1)];
	//avpicture_fill((AVPicture *)pFrameRGB, out_buffer, dst_pix_fmt, 640, 480);
	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer, dst_pix_fmt, sw_frame->width, sw_frame->height, 1);


	SwsContext *sws_ctx = sws_getContext(
		sw_frame->width, sw_frame->height, src_pix_fmt,
		sw_frame->width, sw_frame->height, dst_pix_fmt,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);

	sws_scale(sws_ctx, sw_frame->data, sw_frame->linesize, 0, sw_frame->height, pFrameRGB->data, pFrameRGB->linesize);
	sws_freeContext(sws_ctx);
#endif

#if 1
#if 1
	memcpy(imgColor.data, pFrameRGB->data[0], sw_frame->height * sw_frame->width * 3);
	av_frame_free(&pFrameRGB);
	
	imshow("Color", imgColor);
#else
	memcpy(imgGray.data, sw_frame->data[0], sw_frame->height*sw_frame->width);
	imshow("gray", imgGray);
#endif
	if (cv::waitKey(1000) == 27 /*ESC*/)
	{
		fprintf(stderr, "ESC is pressed\n ");
		exit(0);
	}
#endif

	return 0;
}



int main(int argc, char **argv)
{
	AVFormatContext *input_ctx = NULL;
	AVStream *video_st = NULL;
	AVCodecContext *decoder_ctx = NULL;
	const AVCodec *decoder;
        int frm_counter = 0;

	AVPacket pkt = { 0 };
	AVFrame *frame = NULL, *sw_frame = NULL, *filt_frame = NULL ;

	DecodeContext decode = { NULL };

	AVIOContext *output_ctx = NULL;

	int ret, i;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
		return 1;
	}

	/* open the input file */
	ret = avformat_open_input(&input_ctx, argv[1], NULL, NULL);
	if (ret < 0) {
		fprintf(stderr, "Cannot open input file '%s': ", argv[1]);
		goto finish;
	}

	//init filter structure
	if ((ret = init_filters()) < 0)
		return ret;

    /* find the first H.264 video stream */
    for (i = 0; i < input_ctx->nb_streams; i++) {
        AVStream *st = input_ctx->streams[i];

        if (st->codecpar->codec_id == AV_CODEC_ID_H264 && !video_st)
            video_st = st;
        else
            st->discard = AVDISCARD_ALL;
    }
    if (!video_st) {
        fprintf(stderr, "No H.264 video stream in the input file\n");
        goto finish;
    }

    /* open the hardware device */
    ret = av_hwdevice_ctx_create(&decode.hw_device_ref, AV_HWDEVICE_TYPE_QSV,
                                 "auto", NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Cannot open the hardware device\n");
        goto finish;
    }

    /* initialize the decoder */
    decoder = avcodec_find_decoder_by_name("h264_qsv");
    if (!decoder) {
        fprintf(stderr, "The QSV decoder is not present in libavcodec\n");
        goto finish;
    }

    decoder_ctx = avcodec_alloc_context3(decoder);
    if (!decoder_ctx) {
        ret = AVERROR(ENOMEM);
        goto finish;
    }
    decoder_ctx->codec_id = AV_CODEC_ID_H264;
    if (video_st->codecpar->extradata_size) {
        decoder_ctx->extradata = av_mallocz(video_st->codecpar->extradata_size +
                                            AV_INPUT_BUFFER_PADDING_SIZE);
        if (!decoder_ctx->extradata) {
            ret = AVERROR(ENOMEM);
            goto finish;
        }
        memcpy(decoder_ctx->extradata, video_st->codecpar->extradata,
               video_st->codecpar->extradata_size);
        decoder_ctx->extradata_size = video_st->codecpar->extradata_size;
    }

    decoder_ctx->opaque      = &decode;
    decoder_ctx->get_format  = get_format;

    ret = avcodec_open2(decoder_ctx, NULL, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error opening the decoder: ");
        goto finish;
    }


   filter_ctx->dec_ctx = decoder_ctx;  //arvind

    /* open the output stream */
    ret = avio_open(&output_ctx, argv[2], AVIO_FLAG_WRITE);
    if (ret < 0) {
        fprintf(stderr, "Error opening the output context: ");
        goto finish;
    }

    frame    = av_frame_alloc();
    filt_frame = av_frame_alloc(); // arvind
    sw_frame = av_frame_alloc();
	if (!frame || !sw_frame || !filt_frame) {
		ret = AVERROR(ENOMEM);
		goto finish;
	}

    int got_frame;

  

    /* actual decoding */
    while (ret >= 0) {
        ret = av_read_frame(input_ctx, &pkt);
        if (ret < 0)
            break;

        if (pkt.stream_index == video_st->index)
            //ret = decode_packet(&decode, decoder_ctx, frame, sw_frame, &pkt, output_ctx);
	     ret = decode_packet(decoder_ctx, frame, &got_frame, &pkt);
			
		if (got_frame)
			{
				if (!filter_ctx->initiallized) {
					//init buffer/buffersink and vpp filter here
					ret = init_filter(filter_ctx,
						filter_ctx->dec_ctx, filter_descr);
					if (ret < 0)
						return ret;
				}

				//pts is only used for encoding
				frame->pts = av_frame_get_best_effort_timestamp(frame);
				//display_qsv_frame(frame, sw_frame);

				ret = get_filtered_frame(frame, filt_frame);
				ret = display_qsv_frame(filt_frame, sw_frame);
                                printf( "  -- frm_counter =  %d\n" , frm_counter);
				frm_counter++;
				av_frame_unref(frame);
				av_frame_unref(filt_frame);
			}
        av_packet_unref(&pkt);
    }

    /* flush the decoder */
    pkt.data = NULL;
    pkt.size = 0;
    //ret = decode_packet(&decode, decoder_ctx, frame, sw_frame, &pkt, output_ctx);
     ret = decode_packet(decoder_ctx, frame, &got_frame, &pkt);	
finish:
    if (ret < 0) {
        char buf[1024];
        av_strerror(ret, buf, sizeof(buf));
        fprintf(stderr, "%s\n", buf);
    }

    avformat_close_input(&input_ctx);

    av_frame_free(&frame);
    av_frame_free(&sw_frame);

    avcodec_free_context(&decoder_ctx);

    av_buffer_unref(&decode.hw_device_ref);

    avio_close(output_ctx);

    return ret;
}

