#include "NV_Encoder.h"


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

NV_Encoder::NV_Encoder(en_EncType &encType):H264_Encoder(encType)
{

  
}

NV_Encoder::~NV_Encoder() {


}



void NV_Encoder::encodeFrame(AVCodecContext *dec_ctx,  AVFrame *frame , int w , int h )
{
    AVFrame *tmpFrame = frame;
    
    if(lastFrormat)
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
            {
                flags = AV_BUFFERSRC_FLAG_KEEP_REF; 
                
                c->hw_frames_ctx = av_buffer_ref(dec_ctx->hw_frames_ctx);
                if (!c->hw_frames_ctx) {
                   fprintf(stderr, "Failed to open hw_frames_ctx \n");
                   return;
                }

                c->pix_fmt   = AV_PIX_FMT_CUDA ;//AV_PIX_FMT_VAAPI;
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
        else if(std::string(c->codec->name) == "h264_nvenc") // ffmpeg -h encoder=h264_nvenc  // ffprobe -i test.264 -show_frames | grep 'pict_type' 
        {   
            av_opt_set(c->priv_data, "profile", "high", 0);   // main or 
             
            av_opt_set(c->priv_data, "preset", "fast", 0);
            av_opt_set(c->priv_data, "tune", "ll", 0); 
        }
        

        if ((ret = avcodec_open2(c, codec, NULL)) < 0) {
            fprintf(stderr, "Failed to open encode codec. Error code: \n");
            return;
        }
        
        
        init_filters(dec_ctx);
     

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
        
        if (( av_hwframe_get_buffer(c->hw_frames_ctx, hw_frame, 0)) < 0) {
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


