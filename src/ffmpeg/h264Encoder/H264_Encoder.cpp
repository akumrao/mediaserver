#include "H264_Encoder.h"

H264_Encoder::H264_Encoder(h264_encoder_callback frameCallback, void* user) 
  :codec(NULL)
  ,c(NULL)
  ,fp(NULL)
  ,frame(nullptr)
  ,cb_frame(frameCallback)
  ,cb_user(user)

{
  avcodec_register_all();
}

H264_Encoder::~H264_Encoder() {



  if(c) {
    avcodec_close(c);
    av_free(c);
    c = NULL;
  }

  if(frame) {
    av_free(frame);
    frame = NULL;
  }

  if(fp) {
    fclose(fp);
    fp = NULL;
  }

  cb_frame = NULL;
  cb_user = NULL;

}

bool H264_Encoder::load(std::string filename, int fps, int width, int height) {
    
       
    codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* put sample parameters */
  //  c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = width;
    c->height = height;
    /* frames per second */
    c->time_base = (AVRational){1, fps};
    c->framerate = (AVRational){fps, 1};

    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    c->gop_size = fps;
  //  c->max_b_frames = 1;
    c->max_b_frames = 0;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
   
   // c->color_range = AVCOL_RANGE_JPEG;
    
    
  //  c->bit_rate = config.target_bps * 0.7;
 //   c->rc_max_rate = config.target_bps * 0.85;
  //  c->rc_min_rate = config.target_bps * 0.1;
  //  c->rc_buffer_size = config.target_bps * 2;
    
     
    if (codec->id == AV_CODEC_ID_H264)
    {
       //av_opt_set(c->priv_data, "preset", "slow", 0);
       av_opt_set(c->priv_data, "preset", "ultrafast", 0);
       av_opt_set(c->priv_data, "tune", "zerolatency", 0); 
    }

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        fprintf(stderr, "Could not open %s\n", filename.c_str());
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;

    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }

    
    

  return true;
}


void H264_Encoder::encodeFrame(uint8_t* ydata, int ysize, uint8_t* udata, int usize, uint8_t* vdata, int vsize) {

  av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

    fflush(stdout);

    /* make sure the frame data is writable */
    ret = av_frame_make_writable(frame);
    if (ret < 0)
        exit(1);

    /* prepare a dummy image */
    /* Y */

   frame->data[0]  = ydata;
   
   frame->data[1]  = udata;
   
   frame->data[2]  = vdata;
    
    
   frame->linesize[0] = ysize;
   frame->linesize[1] = usize;
   frame->linesize[2] =vsize;
            
    

    frame->pts = ++frameCount;

    /* encode the image */
    ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0) {
        fprintf(stderr, "Error encoding frame\n");
        exit(1);
    }

    if (got_output) {
        printf("Write frame %3d (size=%5d)\n", frameCount, pkt.size);
        fwrite(pkt.data, 1, pkt.size, fp);
        av_packet_unref(&pkt);
    }
    

    /* get the delayed frames */
    
}






void H264_Encoder::encodeFrame() {

  av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

    fflush(stdout);

    /* make sure the frame data is writable */
    ret = av_frame_make_writable(frame);
    if (ret < 0)
        exit(1);

    frame->pts = ++frameCount;
   /* prepare a dummy image */
        /* Y */
        for (int y = 0; y < c->height; y++) {
            for (x = 0; x < c->width; x++) {
                frame->data[0][y * frame->linesize[0] + x] = x + y + frameCount * 3;
            }
        }

        /* Cb and Cr */
        for (int y = 0; y < c->height/2; y++) {
            for (x = 0; x < c->width/2; x++) {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + frameCount * 2;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + frameCount * 5;
            }
        }
            
    

    

    /* encode the image */
    ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0) {
        fprintf(stderr, "Error encoding frame\n");
        exit(1);
    }

    if (got_output) {
        printf("Write frame %3d (size=%5d)\n", frameCount, pkt.size);
        fwrite(pkt.data, 1, pkt.size, fp);
        av_packet_unref(&pkt);
    }
    

    /* get the delayed frames */
    
}

