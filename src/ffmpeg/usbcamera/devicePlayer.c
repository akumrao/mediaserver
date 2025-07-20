
/*=============================================================================
 * # FileName: read_device.c
 * # Desc: use ffmpeg read a frame data from v4l2, and encode to H264
 *  v4l2-ctl -d /dev/video0 --list-formats-ext
 * ffmpeg -f video4linux2 -r 25 -s 960x720 -i /dev/video0 out.mp4
 * =============================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>



#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>



#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#define LOOP_NUM 1000
//#define OUT_WIDTH 960
//#define OUT_HEIGHT 720
//#define OUT_FPS 25

#define STREAM_FRAME_RATE 25 /* images/s Here you can set the frame rate according to the camera’s acquisition speed*/


char* input_name = "video4linux2";
char* file_name = "/dev/video0";

struct timeval time_val;
float time_start;
float time_end;

AVFormatContext *fmtCtx = NULL;
AVCodecContext *pCodecCtx= NULL;
AVCodecContext *outCodecCtx= NULL;
AVPacket *packet= NULL;
struct SwsContext *sws_ctx= NULL;
uint8_t * src_data[4];
uint8_t * dst_data[4];
int src_linesize[4];
int dst_linesize[4];
int y_size = 0;
AVFrame *outFrame= NULL;
AVStream *video_st= NULL;
int64_t loop = 0;
int got_picture=0;
AVPacket outpkt;
AVFormatContext *outFormatCtx= NULL;
 uint8_t *picture_buf = NULL;
 
float get_diff_time(struct timeval* start, int update) {
    float dt;
    struct timeval now;
    gettimeofday(&now, NULL);
    dt = (float) (now.tv_sec - start->tv_sec);
    dt += (float) (now.tv_usec - start->tv_usec) * 1e-6;

    if (update == 1)
    {
        start->tv_sec = now.tv_sec;
        start->tv_usec = now.tv_usec;
    }

    return dt;
}

int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index) {
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities & AV_CODEC_CAP_DELAY))
        return 0;

    while (1)
    {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
                NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame)
        {
            ret = 0;
            break;
        }
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
        ret = av_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}


void read_device()
{
  //  while (loop++ < LOOP_NUM)
    
    if(av_read_frame(fmtCtx, packet)>=0)
    {   
       
          
       // if(av_compare_ts(loop, outCodecCtx->time_base, STREAM_DURATION, (AVRational){1,1})>=0)  
         //   return ;  
        
        if( packet->size)
        {
            memcpy(src_data[0], packet->data, packet->size);
            sws_scale(sws_ctx, src_data, src_linesize, 0, pCodecCtx->height, dst_data, dst_linesize);


            outFrame->data[0] = dst_data[0];
            outFrame->data[1] = dst_data[0] + y_size;
            outFrame->data[2] = dst_data[0] + y_size * 5 / 4;

            outFrame->pts =  loop++; 
            int ret = avcodec_encode_video2(outCodecCtx, &outpkt, outFrame, &got_picture);
            if (ret < 0)
            {
                printf("Failed to encode! \n");
                return -1;
            }

            if (got_picture == 1)
            {
                outpkt.stream_index = video_st->index;
                ret = av_write_frame(outFormatCtx, &outpkt);
                av_free_packet(&outpkt);
            }
        }
        else
        {
            usleep(10000);  
        }
        
        
         av_packet_unref(packet);  
    }
}

void exitdevice()
{
    time_end = get_diff_time(&time_val, 0);
   // printf("\n\nencoder %d frame spend time = %f \n\n", loop, time_end);

    int ret = flush_encoder(outFormatCtx, 0);
    if (ret < 0)
    {
        printf("Flushing encoder failed\n");
        return -1;
    }


    av_write_trailer(outFormatCtx);
    if (video_st)
    {
        avcodec_close(video_st->codec);
        av_free(outFrame);
        av_free(picture_buf);
    }
    avio_close(outFormatCtx->pb);
    avformat_free_context(outFormatCtx);

    av_free_packet(packet);
    av_freep(&dst_data[0]);
    sws_freeContext(sws_ctx);
    avformat_close_input(&fmtCtx);
}

void captureFrame(void) {
   
    AVInputFormat *inputFmt = NULL;
    
 
    //AVCodec *pCodec= NULL;
   
    FILE *fp = NULL;
    int i;
    int ret;
    int videoindex;

    enum AVPixelFormat dst_pix_fmt = AV_PIX_FMT_YUV420P;




    inputFmt = av_find_input_format(input_name);

    if (inputFmt == NULL)
    {
        printf("can not find_input_format\n");
        return;
    }
    
    
    
    fmtCtx = avformat_alloc_context();
    if (!fmtCtx)
    {
      av_log(0, AV_LOG_ERROR, "Cannot allocate input format (Out of memory?)\n");
      exit(1);
    }

    // Enable non-blocking mode
    //fmtCtx->flags |= AVFMT_FLAG_NONBLOCK;
    
        // Enable non-blocking mode
  //  fmtCtx->flags |= AVFMT_FLAG_NONBLOCK;
    
     AVDictionary *options = NULL;

    // framerate needs to set before opening the v4l2 device
    //av_dict_set(&options, "video_size", "960x720", 0);
    char cfps[12];
    snprintf(cfps, 12,"%d",STREAM_FRAME_RATE);

    av_dict_set(&options, "framerate", cfps, 0);

    // This will not work if the camera does not support h264. In that case
    // remove this line. I wrote this for Raspberry Pi where the camera driver
    // can stream h264.
   // av_dict_set(&options, "input_format", "YUY2", 0);


    if (avformat_open_input(&fmtCtx, file_name, inputFmt, &options) < 0)
    {
        printf("can not open_input_file\n");
        return;
    }

    av_dump_format(fmtCtx, 0, file_name, 0);


    videoindex = -1;
    for (i = 0; i < fmtCtx->nb_streams; i++)
        if (fmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    if (videoindex == -1)
    {
        printf("Didn't find a video stream.\n");
        return -1;
    }

    pCodecCtx = fmtCtx->streams[videoindex]->codec;
   // pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

 
    printf("picture width = %d \n", pCodecCtx->width);
    printf("picture height = %d \n", pCodecCtx->height);
    printf("Pixel Format = %d \n", pCodecCtx->pix_fmt);
    
   // fps = pCodecCtx->framerate.num;
    
    

    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,pCodecCtx->width, pCodecCtx->height , dst_pix_fmt,
            SWS_BILINEAR, NULL, NULL, NULL);

   av_image_alloc(src_data, src_linesize, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 16);
   av_image_alloc(dst_data, dst_linesize, pCodecCtx->width, pCodecCtx->height, dst_pix_fmt, 1);

    packet = (AVPacket *) av_malloc(sizeof (AVPacket));

    /* set out format */
    
    AVOutputFormat *outfmt;
    
    

    AVCodec *outCodec;

    

   
    char *out_file = "ds.h264";
    int picture_size;
    
  
   

    outFormatCtx = avformat_alloc_context();
    outfmt = av_guess_format(NULL, out_file, NULL);
    outFormatCtx->oformat = outfmt;
    if (avio_open(&outFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0)
    {
        printf("Failed to open output file! \n");
        return -1;
    }
    video_st = avformat_new_stream(outFormatCtx, 0);
    if (video_st == NULL)
    {
        printf(" creat new stream err \n ");
        return -1;
    }

    outCodecCtx = video_st->codec;
    outCodecCtx->codec_id = outfmt->video_codec;
    outCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    outCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    outCodecCtx->width = pCodecCtx->width;
    outCodecCtx->height = pCodecCtx->height;
    outCodecCtx->bit_rate = 400000;
    outCodecCtx->gop_size = 64;

    outCodecCtx->time_base.num = 1;
    outCodecCtx->time_base.den = STREAM_FRAME_RATE;


    outCodecCtx->qmin = 10;
    outCodecCtx->qmax = 51;
    outCodecCtx->max_b_frames = 0;

    outCodecCtx->profile = FF_PROFILE_H264_BASELINE;
    outCodecCtx->level = 31;
    
    
    AVDictionary *param = NULL;
    if (outCodecCtx->codec_id == AV_CODEC_ID_H264)
    {
      //  av_dict_set(&param, "preset", "faster", 0);
      //  av_dict_set(&param, "tune", "zerolatency", 0); 
        
        
        av_opt_set(&param, "profile", "main", 0);  // main or
        av_opt_set(&param, "preset", "medium", 0);  // fast is  baseline
        av_opt_set(&param, "tune", "zerolatency", 0);
        
        
    }

    av_dump_format(outFormatCtx, 0, out_file, 1);

    outCodec = avcodec_find_encoder(outCodecCtx->codec_id);
    if (!outCodec)
    {
        printf("Can not find encoder! \n");
        return -1;
    }

    if (avcodec_open2(outCodecCtx, outCodec, &param) < 0)
    {
        printf("Failed to open encoder! \n");
        return -1;
    }

    outFrame = av_frame_alloc();
    picture_size = avpicture_get_size(outCodecCtx->pix_fmt, outCodecCtx->width, outCodecCtx->height);
    picture_buf = (uint8_t *) av_malloc(picture_size);
    avpicture_fill((AVPicture *) outFrame, picture_buf, outCodecCtx->pix_fmt, outCodecCtx->width, outCodecCtx->height);
    outFrame->format = outCodecCtx->pix_fmt;
    outFrame->width = outCodecCtx->width;
    outFrame->height = outCodecCtx->height;
    avformat_write_header(outFormatCtx, NULL);
    av_new_packet(&outpkt, picture_size);
    y_size = outCodecCtx->width * outCodecCtx->height;

    time_start = get_diff_time(&time_val, 1);
    while (loop < LOOP_NUM)
    {
        read_device();
        
//        av_read_frame(fmtCtx, packet);
//        memcpy(src_data[0], packet->data, packet->size);
//        sws_scale(sws_ctx, src_data, src_linesize, 0, pCodecCtx->height, dst_data, dst_linesize);
//
//
//        outFrame->data[0] = dst_data[0];
//        outFrame->data[1] = dst_data[0] + y_size;
//        outFrame->data[2] = dst_data[0] + y_size * 5 / 4;
//
//        outFrame->pts = (loop - 1)*(video_st->time_base.den) / ((video_st->time_base.num)*25);
//        ret = avcodec_encode_video2(outCodecCtx, &outpkt, outFrame, &got_picture);
//        if (ret < 0)
//        {
//            printf("Failed to encode! \n");
//            return -1;
//        }
//
//        if (got_picture == 1)
//        {
//            outpkt.stream_index = video_st->index;
//            ret = av_write_frame(outFormatCtx, &outpkt);
//            av_free_packet(&outpkt);
//        }
    }


    exitdevice();
}





int initReaddevice(void) {
    av_register_all();
    avcodec_register_all();
    avdevice_register_all();
    captureFrame();
    return 0;
}


int main(void) {
    initReaddevice();
    return 0;
}





