
/*=============================================================================
 * # FileName: read_device.c
 * # Desc: use ffmpeg read a frame data from v4l2, and encode to H264
 *  v4l2-ctl -d /dev/video0 --list-formats-ext
 * ffmpeg -f video4linux2 -r 25 -s 960x720 -i /dev/video0 out.mp4
 * =============================================================================*/


#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include <math.h>  
#include <libavutil/avassert.h>  
#include <libavutil/channel_layout.h>  
#include <libavutil/opt.h>  
#include <libavutil/mathematics.h>  
#include <libavutil/timestamp.h>  
#include <libavformat/avformat.h>  
#include <libswscale/swscale.h>  
#include <libswresample/swresample.h>  
 
#define STREAM_DURATION 20.0 /*Duration of recorded video in seconds*/
#define STREAM_FRAME_RATE 25 /* images/s Here you can set the frame rate according to the camera’s acquisition speed*/
#define STREAM_PIX_FMT AV_PIX_FMT_YUV420P /* default pix_fmt */  
#define SCALE_FLAGS SWS_BICUBIC  
 
//Storage the width and height of the video
int video_width;
int video_height;
 
// Wrapper for a single output AVStream
typedef struct OutputStream
{  
  AVStream *st;  
  AVCodecContext *enc;  
  /*Points for next frame*/  
  int64_t next_pts;  
  int samples_count;  
  AVFrame *frame;  
  AVFrame *tmp_frame;  
  float t, tincr, tincr2;  
  struct SwsContext *sws_ctx;  
  struct SwrContext *swr_ctx;  
}OutputStream;  
 
 
typedef struct IntputDev
{  
  AVCodecContext *pCodecCtx;  
  AVCodec *pCodec;  
  AVFormatContext *v_ifmtCtx;  
  int videoindex;  
  struct SwsContext *img_convert_ctx;  
  AVPacket *in_packet;  
  AVFrame *pFrame;  
}IntputDev;  
 
static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)  
{  
  AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;  
  printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",  
       av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),  
       av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),  
       av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),  
       pkt->stream_index);  
}  
 
static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)  
{  
  /* Rescale output packet timestamp values ​​from codec to stream timebase */  
  av_packet_rescale_ts(pkt, *time_base, st->time_base);  
  pkt->stream_index = st->index;  
 
  /*Write compressed frames to media files. */  
  log_packet(fmt_ctx, pkt);  
  return av_interleaved_write_frame(fmt_ctx, pkt);  
}  
 
/*Add output stream. */  
static void add_stream(OutputStream *ost, AVFormatContext *oc,AVCodec **codec,enum AVCodecID codec_id)  
{  
  AVCodecContext *c;  
  int i;  
  /* find the encoder */
 
  *codec = avcodec_find_encoder(codec_id);  
  if (!(*codec))
  {  
    fprintf(stderr, "Could not find encoder for '%s'\n",  
        avcodec_get_name(codec_id));  
    exit(1);  
  }  
  ost->st = avformat_new_stream(oc, NULL);  
 
  if (!ost->st) {  
    fprintf(stderr, "Could not allocate stream\n");  
    exit(1);  
  }  
  ost->st->id = oc->nb_streams-1;  
  c = avcodec_alloc_context3(*codec);  
  if (!c) {  
    fprintf(stderr, "Could not alloc an encoding context\n");  
    exit(1);  
  }  
  ost->enc = c;   
  switch((*codec)->type)
  {  
    case AVMEDIA_TYPE_AUDIO:  
      c->sample_fmt = (*codec)->sample_fmts?  
        (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;  
      c->bit_rate = 64000;  
      c->sample_rate = 44100;  
      if ((*codec)->supported_samplerates) {  
        c->sample_rate = (*codec)->supported_samplerates[0];  
        for (i = 0; (*codec)->supported_samplerates[i]; i++) {  
          if ((*codec)->supported_samplerates[i] == 44100)  
            c->sample_rate = 44100;  
        }  
      }  
      c->channels = av_get_channel_layout_nb_channels(c->channel_layout);  
      c->channel_layout = AV_CH_LAYOUT_STEREO;  
      if ((*codec)->channel_layouts) {  
        c->channel_layout = (*codec)->channel_layouts[0];  
        for (i = 0; (*codec)->channel_layouts[i]; i++) {  
          if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)  
            c->channel_layout = AV_CH_LAYOUT_STEREO;  
        }  
      }  
      c->channels = av_get_channel_layout_nb_channels(c->channel_layout);  
      ost->st->time_base = (AVRational){ 1, c->sample_rate };  
      break;  
    
    case AVMEDIA_TYPE_VIDEO:  
      c->codec_id = codec_id;  
      c->bit_rate = 2500000; //Average bit rate, the default value of the example code is 400000
      /* Resolution must be a multiple of 2. */  
      c->width=video_width;  
      c->height=video_height;  
      /*Time base: This is the basic unit of time (in seconds)
       *Indicates the frame timestamp in it. For fixed fps content,
       *timebase should be 1/framerate and timestamp increment should be
       * equals 1. */  
      ost->st->time_base = (AVRational){1,STREAM_FRAME_RATE}; //Frame rate setting
      c->time_base = ost->st->time_base;  
      c->gop_size = 12; /* Emit one intra-frame at most every twelve frames*/  
      c->pix_fmt = STREAM_PIX_FMT;  
      if(c->codec_id == AV_CODEC_ID_MPEG2VIDEO)
      {  
        /* Just for testing, we also added B frames*/  
        c->max_b_frames = 2;  
      }  
      if(c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
      {  
        /*Needs to avoid using macroblocks where some of the coefficients overflow.
         *This does not happen with normal videos because
         *The movement of the chroma plane does not match the luma plane. */
        c->mb_decision = 2;  
      }
    break;  
    
    default:  
      break;  
  }  
  
  /* Some formats expect stream headers to be separated. */  
  if (oc->oformat->flags & AVFMT_GLOBALHEADER)  
    c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;    
}  
 
static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)  
{  
  AVFrame *picture;  
  int ret;  
  picture = av_frame_alloc();  
  if (!picture)  
    return NULL;  
  picture->format = pix_fmt;  
  picture->width = width;  
  picture->height = height;  
  
  /* Allocate buffer for frame data*/  
  ret = av_frame_get_buffer(picture, 32);  
  if(ret<0)
  {  
    fprintf(stderr, "Could not allocate frame data.\n");  
    exit(1);  
  }  
  return picture;  
}  
  
static void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)  
{  
  int ret;  
  AVCodecContext *c = ost->enc;  
  AVDictionary *opt = NULL;  
  
  av_dict_copy(&opt, opt_arg, 0);  
  
  /* open the codec */  
  ret = avcodec_open2(c, codec, &opt);  
  av_dict_free(&opt);  
  if (ret < 0)
  {  
    fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));  
    exit(1);  
  }  
  
  /* Allocate and initialize reusable frameworks*/  
  ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);  
  if (!ost->frame)
  {  
    fprintf(stderr, "Could not allocate video frame\n");  
    exit(1);  
  }  
  printf("ost->frame alloc success fmt=%dw=%dh=%d\n",c->pix_fmt,c->width, c->height);  
  
  
  /*If the output format is not YUV420P, it is temporary YUV420P
  *Pictures are also required. and then convert it to the desired
  *Output format. */
  ost->tmp_frame = NULL;  
  if(c->pix_fmt != AV_PIX_FMT_YUV420P)
  {  
    ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);  
    if (!ost->tmp_frame)
    {  
      fprintf(stderr, "Could not allocate temporary picture\n");  
      exit(1);  
    }  
  }  
  
  /* Copy stream parameters to multiplexer */  
  ret=avcodec_parameters_from_context(ost->st->codecpar, c);  
  if(ret<0)
  {  
    fprintf(stderr, "Could not copy the stream parameters\n");  
    exit(1);  
  }  
}
 
/*
  *Encode a video frame
  *Returns 1 after encoding is completed, otherwise returns 0
  */  
static int write_video_frame(AVFormatContext *oc, OutputStream *ost,AVFrame *frame)  
{  
  int ret;  
  AVCodecContext *c;  
  int got_packet=0;  
  AVPacket pkt={0};  
  if(frame==NULL)  
    return 1;  
  c = ost->enc;  
  av_init_packet(&pkt);  
  /* Encode image */  
  ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);  
  if(ret<0)
  {  
    fprintf(stderr, "Error encoding video frame: %s\n", av_err2str(ret));  
    exit(1);  
  }  
  printf("--------------video- pkt.pts=%s\n",av_ts2str(pkt.pts));  
  printf("----st.num=%d st.den=%d codec.num=%d codec.den=%d---------\n",ost->st-> time_base.num,ost->st->time_base.den,  
      c->time_base.num,c->time_base.den);     
  if(got_packet)
  {  
    ret = write_frame(oc, &c->time_base, ost->st, &pkt);  
  }else
  {  
    ret = 0;  
  }  
  if(ret<0)
  {  
    fprintf(stderr, "Error while writing video frame: %s\n", av_err2str(ret));  
    exit(1);  
  }  
  return (frame || got_packet) ? 0 : 1;  
}  
  
  
static AVFrame *get_video_frame(OutputStream *ost,IntputDev* input,int *got_pic)  
{  
  int ret, got_picture;  
  AVCodecContext *c = ost->enc;  
  AVFrame * ret_frame=NULL;  
  if(av_compare_ts(ost->next_pts, c->time_base,STREAM_DURATION, (AVRational){1,1})>=0)  
    return NULL;  
  
  /*When we pass the frame to the encoder, it may keep a reference to it
  *Internally, make sure we don't cover it here*/
  if (av_frame_make_writable(ost->frame)<0)  
    exit(1);  
  if(av_read_frame(input->v_ifmtCtx, input->in_packet)>=0)
  {  
    if(input->in_packet->stream_index==input->videoindex)
    {  
      ret = avcodec_decode_video2(input->pCodecCtx, input->pFrame, &got_picture, input->in_packet);  
      *got_pic=got_picture;  
      if(ret<0)
      {  
        printf("Decode Error.\n");  
        av_packet_unref(input->in_packet);  
        return NULL;  
      }  
      if(got_picture)
      {  
        sws_scale(input->img_convert_ctx, (const unsigned char* const*)input->pFrame->data, input->pFrame->linesize, 0, input->pCodecCtx->height, ost->frame->data, ost ->frame->linesize);  
        ost->frame->pts =ost->next_pts++;  
        ret_frame= ost->frame;    
      }  
    }  
    av_packet_unref(input->in_packet);  
  }  
  return ret_frame;  
}  

static void close_stream(AVFormatContext *oc, OutputStream *ost)  
{  
  avcodec_free_context(&ost->enc);  
  av_frame_free(&ost->frame);  
  av_frame_free(&ost->tmp_frame);  
  sws_freeContext(ost->sws_ctx);  
  swr_free(&ost->swr_ctx);  
}  
 
/*
Collect camera data and encode it into MP4 video
*/  
int main(int argc, char **argv)  
{  
  OutputStream video_st = { 0 }, audio_st = { 0 };  
  const char *filename;  
  const char* dev;
  AVOutputFormat *fmt;  
  AVFormatContext *oc;  
  AVCodec *audio_codec, *video_codec;  
  int ret;  
  int have_video = 0, have_audio = 0;  
  int encode_video = 0, encode_audio = 0;  
  AVDictionary *opt = NULL;  
  int i;  
  
  

  
  if(argc<3)
  {  
    //./app /dev/video0 123.mp4
    
   filename = "test.mp4";
   dev= "/dev/video0";
  } 
  else
  {
    dev= argv[1]; 
    filename = argv[2]; 
  }
  
   
  printf("Currently stored video file name: %s\n",filename);
  /*Assign output media environment*/
  avformat_alloc_output_context2(&oc, NULL, NULL, filename);  
  if(!oc)
  {  
    printf("The output format cannot be inferred from the file extension: use MPEG.\n");  
    avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);  
  }  
  if(!oc)return 1;  
  //Add camera---------------------------------
  IntputDev video_input={0};  
  AVCodecContext *pCodecCtx;  
  AVCodec *pCodec;  
  AVFormatContext *v_ifmtCtx;  
  avdevice_register_all();  
  v_ifmtCtx = avformat_alloc_context();  
  //Specify camera information under Linux  
  
  // Enable non-blocking mode
  //v_ifmtCtx->flags |= AVFMT_FLAG_NONBLOCK;
    
  AVDictionary *options = NULL;

    // framerate needs to set before opening the v4l2 device
 // av_dict_set(&options, "video_size", "960x720", 0);
  
  char cfps[12];
  snprintf(cfps, 12,"%d",STREAM_FRAME_RATE);

  av_dict_set(&options, "framerate", cfps, 0);
    // This will not work if the camera does not support h264. In that case
    // remove this line. I wrote this for Raspberry Pi where the camera driver
    // can stream h264.
   // av_dict_set(&options, "input_format", "YUY2", 0);
   //av_dict_set(&options, "video_size", "960x720", 0);
  
  AVInputFormat *ifmt=av_find_input_format("video4linux2");  
  if(avformat_open_input(&v_ifmtCtx,dev,ifmt,&options)!=0)
  {  
    printf("Unable to open input stream.%s\n",dev);  
    return -1;  
  }
  if(avformat_find_stream_info(v_ifmtCtx,NULL)<0)  
  {  
    printf("Stream information not found.\n");  
    return -1;  
  }
  int videoindex=-1;  
  for(i=0; i<v_ifmtCtx->nb_streams; i++)   
  if(v_ifmtCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)  
  {  
    videoindex=i;  
    printf("videoindex=%d\n",videoindex);
    break;  
  }
  if(videoindex==-1)  
  {  
    printf("Video stream not found.\n");  
    return -1;  
  }  
  pCodecCtx=v_ifmtCtx->streams[videoindex]->codec;
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);  
  if(pCodec==NULL)  
  {  
    printf("Codec not found.\n");  
    return -1;  
  }  
  if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)  
  {  
    printf("Unable to open codec.\n");  
    return -1;  
  }  
  
  AVFrame *pFrame;  
  pFrame=av_frame_alloc();  
  //pFrameYUV=av_frame_alloc();
  
  //unsigned char *out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height,16));  
  //av_image_fill_arrays((AVPicture *)pFrameYUV->data,(AVPicture *)pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height,16);  
  printf("Camera size (WxH): %dx %d \n",pCodecCtx->width, pCodecCtx->height);  
  video_width=pCodecCtx->width;
  video_height=pCodecCtx->height;
  struct SwsContext *img_convert_ctx;  
  img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);   
  AVPacket *in_packet=(AVPacket *)av_malloc(sizeof(AVPacket));  
  video_input.img_convert_ctx=img_convert_ctx;  
  video_input.in_packet=in_packet;  
  video_input.pCodecCtx=pCodecCtx;  
  video_input.pCodec=pCodec;  
  video_input.v_ifmtCtx=v_ifmtCtx;  
  video_input.videoindex=videoindex;  
  video_input.pFrame=pFrame;  
 // video_input.pFrameYUV=pFrameYUV;  
    //--------------------------End of adding camera
  fmt=oc->oformat;  
  /*Add audio and video streams using the default format codec and initialize the codec. */
  printf("fmt->video_codec = %d\n", fmt->video_codec);  
  if(fmt->video_codec != AV_CODEC_ID_NONE)
  {  
    add_stream(&video_st,oc,&video_codec,fmt->video_codec);  
    have_video=1;  
    encode_video=1;  
  }  
  /* Now that all parameters have been set, the audio and video codecs can be turned on and the necessary encoding buffers allocated. */
  if(have_video)open_video(oc, video_codec, &video_st, opt);  
  av_dump_format(oc,0,filename,1);  
  /* Open output file (if necessary) */  
  if(!(fmt->flags & AVFMT_NOFILE))
  {  
    ret=avio_open(&oc->pb,filename,AVIO_FLAG_WRITE);  
    if(ret<0)
    {  
      fprintf(stderr, "Cannot open '%s': %s\n", filename,av_err2str(ret));  
      return 1;  
    }  
  }  
  /* Write stream header (if any)*/  
  ret=avformat_write_header(oc, &opt);  
  if(ret<0)
  {  
    fprintf(stderr, "An error occurred while opening the output file: %s\n",av_err2str(ret));  
    return 1;  
  }  
  int got_pic;  
  while(encode_video)
  {  
    /*Select the stream to be encoded*/    
    AVFrame *frame=get_video_frame(&video_st,&video_input,&got_pic);  
    if(!got_pic)  
    {  
      usleep(10000);  
      continue;  
    }
    encode_video=!write_video_frame(oc,&video_st,frame);  
  }  
  av_write_trailer(oc);  
  sws_freeContext(video_input.img_convert_ctx);  
  avcodec_close(video_input.pCodecCtx);  
  av_free(video_input.pFrame);      
  avformat_close_input(&video_input.v_ifmtCtx);
  /*Turn off each codec*/  
  if (have_video)close_stream(oc, &video_st);
  /*Close output file*/
  if (!(fmt->flags & AVFMT_NOFILE))avio_closep(&oc->pb);  
  /*release stream*/  
  avformat_free_context(oc);
  return 0;  
}
