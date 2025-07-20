#include "QSV_Encoder.h"


#include "rtc_base/ref_counted_object.h"
#include "rtc_base/atomic_ops.h"
#include <chrono>
#include "base/platform.h"
#include "tools.h"
#include "common_video/h264/sps_parser.h"
#include "common_video/h264/h264_common.h"

#include "H264Framer.h"
#include "base/logger.h"

using namespace base;

//#define _DUMPFILE1 1

/*
extern "C" {
#include <libavutil/hwcontext_d3d11va.h>
}
*/

namespace base {
    namespace web_rtc {
QSV_Encoder::QSV_Encoder(en_EncType &encType):H264_Encoder(encType)
{

  
}

QSV_Encoder::~QSV_Encoder() {


}


int QSV_Encoder::init_filter(FilteringContext* fctx, AVCodecContext *dec_ctx,   AVCodecContext *enc_ctx, const char *filter_spec)
{
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc = NULL;
    const AVFilter *buffersink = NULL;
    AVFilterContext *buffersrc_ctx = NULL;
    AVFilterContext *buffersink_ctx = NULL;
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVFilterGraph *filter_graph = avfilter_graph_alloc();
     AVBufferRef *frames_ref = NULL;
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
        buffersrc = avfilter_get_by_name("buffer");
        buffersink = avfilter_get_by_name("buffersink");
        if (!buffersrc || !buffersink) {
            av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

         if( encType == EN_QUICKSYNC)
         {
            snprintf(args, sizeof (args),
                "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d"
                ":frame_rate=%d/%d",
                dec_ctx->width, dec_ctx->height, AV_PIX_FMT_QSV, // dec_ctx->pix_fmt,
                dec_ctx->time_base.num, dec_ctx->time_base.den,
                dec_ctx->sample_aspect_ratio.num,
                dec_ctx->sample_aspect_ratio.den,
                dec_ctx->framerate.num, dec_ctx->framerate.den);
        }
        else
        {
	      snprintf(args, sizeof(args),
                "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt, //AV_PIX_FMT_QSV ,
                dec_ctx->time_base.num, dec_ctx->time_base.den,
                dec_ctx->sample_aspect_ratio.num,
                dec_ctx->sample_aspect_ratio.den);

	}

        ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                args, NULL, filter_graph);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
            goto end;
        }
        
        if(  encType)
        {
            AVBufferSrcParameters *par = av_buffersrc_parameters_alloc();

            if (!par)
                return AVERROR(ENOMEM);
            memset(par, 0, sizeof(*par));
            par->format = AV_PIX_FMT_NONE;


            par->hw_frames_ctx = dec_ctx->hw_frames_ctx ;//  ifilter->hw_frames_ctx;
            ret = av_buffersrc_parameters_set(buffersrc_ctx, par);
            if (ret < 0)
                goto end;
            av_freep(&par);
        }

        
        

        ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                NULL, NULL, filter_graph);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "pix_fmts",
                (uint8_t*)&enc_ctx->pix_fmt, sizeof(enc_ctx->pix_fmt),
                AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
            goto end;
        }
    } else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        buffersrc = avfilter_get_by_name("abuffer");
        buffersink = avfilter_get_by_name("abuffersink");
        if (!buffersrc || !buffersink) {
            av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        if (!dec_ctx->channel_layout)
            dec_ctx->channel_layout =
                av_get_default_channel_layout(dec_ctx->channels);
        snprintf(args, sizeof(args),
                "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
                dec_ctx->time_base.num, dec_ctx->time_base.den, dec_ctx->sample_rate,
                av_get_sample_fmt_name(dec_ctx->sample_fmt),
                dec_ctx->channel_layout);
        ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                args, NULL, filter_graph);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
            goto end;
        }

        ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                NULL, NULL, filter_graph);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer sink\n");
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "sample_fmts",
                (uint8_t*)&enc_ctx->sample_fmt, sizeof(enc_ctx->sample_fmt),
                AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "channel_layouts",
                (uint8_t*)&enc_ctx->channel_layout,
                sizeof(enc_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "sample_rates",
                (uint8_t*)&enc_ctx->sample_rate, sizeof(enc_ctx->sample_rate),
                AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
            goto end;
        }
    } else {
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    /* Endpoints for the filter graph. */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    if (!outputs->name || !inputs->name) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_spec,
                    &inputs, &outputs, NULL)) < 0)
        goto end;
    
    
    
    if(  encType)     
    for (int i = 0; i < filter_graph->nb_filters; i++)
    {
        filter_graph->filters[i]->hw_device_ctx =   av_buffer_ref(avcodec_hw_device_ctx);  // av_buffer_ref(enc_ctx->hw_device_ctx );
           
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
    
    if( encType == EN_QUICKSYNC)
    { 
	frames_ref = av_buffersink_get_hw_frames_ctx(fctx->buffersink_ctx);
	if (frames_ref &&
	((AVHWFramesContext*)frames_ref->data)->format ==  enc_ctx->pix_fmt) {
	// Matching format, will try to use hw_frames_ctx.
	} else {
	  frames_ref = NULL;
	}



	enc_ctx->hw_frames_ctx = av_buffer_ref(frames_ref);
     }


end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}



int QSV_Encoder::init_filters(AVCodecContext *dec_ctx)
{

    unsigned int i;
    int ret;
    
    char filter_spec[512]= {'n','u','l','l','\0'};
      
    filter_ctx = (FilteringContext *)av_malloc_array(1 , sizeof(*filter_ctx));
    if (!filter_ctx)
        return AVERROR(ENOMEM);

    for (i = 0; i < 1; i++) {
        filter_ctx[i].buffersrc_ctx  = NULL;
        filter_ctx[i].buffersink_ctx = NULL;
        filter_ctx[i].filter_graph   = NULL;

      //  if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {

          
          if(!encType)
           snprintf(filter_spec, sizeof(filter_spec),  "scale=%d:%d",   c->width,  c->height); //filter_spec = "scale=w=iw/2:-1";
          else if( encType == EN_NVIDIA )
          {
             //filter_spec =  "scale_cuda=w=iw/2:-1"; //scale_cuda=640:480"
             snprintf(filter_spec, sizeof(filter_spec),  "scale_cuda=%d:%d",   c->width,  c->height);
             
          }
          else  if( encType == EN_QUICKSYNC )
          {
             //snprintf(filter_spec, sizeof(filter_spec),  "scale_qsv=%d:%d",   c->width,  c->height);
             
             snprintf(filter_spec, sizeof(filter_spec),  "vpp_qsv=w=%d:h=%d",   c->width,  c->height);
              
          }
          /*else  if( encType == EN_VAAPI )
          {
             snprintf(filter_spec, sizeof(filter_spec),  "scale_vaapi=%d:%d",   c->width,  c->height);
              
          }*/
         // filter_spec = "drawtext=fontfile=FreeSerif.ttf: text='%{localtime}': x=w-text_w: y=0: fontsize=24: fontcolor=yellow@1.0: box=1: boxcolor=red@1.0";
         // filter_spec = "drawtext=fontfile=FreeSerif.ttf :text='test': x=w-text_w: y=text_h: fontsize=24: fontcolor=yellow@1.0: box=1: boxcolor=red@1.0";


          //filter_spec = "rate=6"; //filter_spec = "null"; /* passthrough (dummy) filter for video */  

        }  



        ret = init_filter(&filter_ctx[i], dec_ctx, c , filter_spec);
        if (ret)
            return ret;

        filter_ctx[i].enc_pkt = av_packet_alloc();
        if (!filter_ctx[i].enc_pkt)
            return AVERROR(ENOMEM);

        filter_ctx[i].filtered_frame = av_frame_alloc();
        if (!filter_ctx[i].filtered_frame)
            return AVERROR(ENOMEM);
    }
    return 0;
}






void QSV_Encoder::encodeFrame(AVCodecContext *dec_ctx,  AVFrame *frame , int w , int h )
{
    AVFrame *tmpFrame = frame;
    
  /* if(lastFrormat)
    {
       if( lastFrormat !=  w + h + frame->format)
       {
           clean();
           
           c = avcodec_alloc_context3(codec);
           if (!c) {
                fprintf(stderr, "Could not allocate video codec context\n");
                exit(1);
           }
           lastFrormat = w + h + frame->format;   
       }
    }
    else
    {
        lastFrormat = w + h + frame->format;
    }
  */  
    
    
    if(!filter_ctx)
    {
        //filter_ctx = new FilteringContext();
        //std::string fileName = "/tmp/test2.264";
        if( !dec_ctx->time_base.num )
        {
            dec_ctx->time_base.num = 1;
            dec_ctx->time_base.den = 60;
            
            dec_ctx->framerate.num = 30;
            dec_ctx->framerate.den = 1; 
        }
        int fr = (dec_ctx->time_base.den )/(dec_ctx->time_base.num*dec_ctx->ticks_per_frame);
        
        
        //load(fileName, fr , frame->width/2,  frame->height/2);
        
        c->pix_fmt   = AV_PIX_FMT_YUV420P ;//AV_PIX_FMT_VAAPI;

         
        if(encType)
        {
           
            if( frame->format == AV_PIX_FMT_NV12)
            {
                if(set_hwframe_ctx(c, avcodec_hw_device_ctx,AV_PIX_FMT_NV12,  frame->width, frame->height) < 0) 
                {
                   printf("avcodec: encode: Failed to set hwframe context.\n");
                   exit(0);
                }
                    
               /*
                if (( av_hwframe_get_buffer(c->hw_frames_ctx, hw_frame, 0)) < 0) {
                   fprintf(stderr, "Hw frame is not initialized");
                    exit(0);
                }
               
                if (!hw_frame->hw_frames_ctx) {

                      fprintf(stderr, "Hw frame is not initialized");
                       exit(0);
                }
		*/
                    
                
            }
            else
            {
                flags = AV_BUFFERSRC_FLAG_KEEP_REF; 
                
                c->hw_frames_ctx = av_buffer_ref(dec_ctx->hw_frames_ctx);
                if (!c->hw_frames_ctx) {
                   fprintf(stderr, "Failed to open hw_frames_ctx \n");
                   return;
                }

                c->pix_fmt    = avcodec_hw_pix_fmt  ; //AV_PIX_FMT_CUDA ;//AV_PIX_FMT_VAAPI;
            }
        }
        /* set AVCodecContext Parameters for encoder, here we keep them stay
         * the same as decoder.
         * xxx: now the sample can't handle resolution change case.
         */
        c->time_base = av_inv_q(dec_ctx->framerate);
 
        c->width     = w ;  //dec_ctx->width;
        c->height    = h;   //dec_ctx->height;
    
        c->bit_rate = 200000;
        
        c->rc_max_rate = 300000;
         
        c->gop_size = 300;
        c->max_b_frames = 0;
       // c->max_b_frames = 3;
        
//        c->level = 40;
//        av_opt_set(c->priv_data, "profile", "main", 0);
        
        c->level = 31;
        
        if(std::string(c->codec->name) == "libx264") // ffmpeg -h encoder=libx264
        {    
            av_opt_set(c->priv_data, "profile", "main", 0);   // main or 
            av_opt_set(c->priv_data, "preset", "medium", 0);  // fast is  baseline 
            av_opt_set(c->priv_data, "tune", "zerolatency", 0); 
            c->thread_count = 1;

        }
        else if(std::string(c->codec->name) == "h264_nvenc" || std::string(c->codec->name) == "h264_qsv" ) // ffmpeg -h encoder=h264_nvenc  // ffprobe -i test.264 -show_frames | grep 'pict_type' 
        {   
            av_opt_set(c->priv_data, "profile", "high", 0);   // main or 
             
            av_opt_set(c->priv_data, "preset", "fast", 0);
            av_opt_set(c->priv_data, "tune", "ll", 0); 
        }
        
        init_filters(dec_ctx);
        if ((ret = avcodec_open2(c, codec, NULL)) < 0) {
            fprintf(stderr, "Failed to open encode codec. Error code: \n");
            return;
        }
        
        
        //init_filters(dec_ctx);
     

//        LayerConfig config;
//
//        config.width = 800;
//        config.height = 640;
//
//        config.max_frame_rate = 5;
//
//        config.key_frame_interval = GOPSIZE;
//
//        config.max_bps = 2000;
//        config.target_bps= 2000;
        
        //   if(x++ == 0 )    
        //   reconfigure(config);
    
    }
    
    
    frame->pts =  decoderCount++;
    
    if (frame->format == AV_PIX_FMT_NV12)
    {
        
        //        if(hw_frame) {
        //          av_frame_free(&hw_frame);
        //          hw_frame = NULL;
        //        }
  
       // hw_frame = av_frame_alloc();
        
        if (( av_hwframe_get_buffer(dec_ctx->hw_frames_ctx, hw_frame, 0)) < 0) {
           fprintf(stderr, "Hw frame is not initialized");
            exit(0);
        }
          
        
        int err;

        if ((err = av_hwframe_transfer_data(hw_frame, frame, 0)) < 0) {
            printf("avcodec: encode: Error while transferring"
                    " frame data to surface."
                    "Error code: \n");
            return;
        }
        
        //av_frame_copy_props(hw_frame, frame);
        
        tmpFrame = hw_frame;
        
    }
   
      
    int ret = filter_encode_write_frame(tmpFrame, 0, flags);
    if (ret < 0)
    {
         fprintf(stderr, "filter_encode_write_frame failed");
    }
    
    return ;
   
   
//    SInfo << "About to Encode frame" << frame->pts ;
     
    av_init_packet(pkt);
    pkt->data = NULL; // packet data will be allocated by the encoder
    pkt->size = 0;
    
    
    ret = avcodec_send_frame(c, tmpFrame);
   

    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(c, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
           // av_packet_unref(pkt);  // do not do it
            return;
        }
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
  
        
       
        //SInfo << "Encoded h264 Frame " <<  frameCount << " size " <<   pkt->size;
        
         ++frameCount;
         
       // fwrite(pkt->data, 1, pkt->size, fp);
        
        #if _DUMPFILE1  
        fwrite(pkt->data, 1, pkt->size, fp);
        #endif
        
        write_frame(pkt);
        
        
        av_packet_unref(pkt);
    }
    
}
}}