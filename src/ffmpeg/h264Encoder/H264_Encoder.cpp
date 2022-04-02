#include "H264_Encoder.h"



AVPixelFormat avcodec_hw_pix_fmt = AV_PIX_FMT_CUDA;
AVPixelFormat avcodec_sw_pix_fmt  = AV_PIX_FMT_YUV420P;

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
    int i, j, found, err;

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


static int set_hwframe_ctx(AVCodecContext *ctx, AVBufferRef *device_ctx,
			   int width, int height)
{
	AVBufferRef *hw_frames_ref;
	AVHWFramesContext *frames_ctx = NULL;
	int err = 0;

	printf("avcodec: encode: create hardware frames.. (%d x %d)\n",
	     width, height);

	if (!(hw_frames_ref = av_hwframe_ctx_alloc(device_ctx))) {
		printf("avcodec: encode: Failed to create hardware"
			" frame context.\n");
		return ENOMEM;
	}


        
	frames_ctx = (AVHWFramesContext *)(void *)hw_frames_ref->data;
	frames_ctx->format    = avcodec_hw_pix_fmt;
        
        ctx->pix_fmt = avcodec_hw_pix_fmt;
        
	frames_ctx->sw_format = avcodec_sw_pix_fmt   ;        
	frames_ctx->width     = width;
	frames_ctx->height    = height;
	frames_ctx->initial_pool_size = 20;

	if ((err = av_hwframe_ctx_init(hw_frames_ref)) < 0) {
		printf("avcodec: encode:"
			" Failed to initialize hardware frame context."
			"Error code: \n");
		av_buffer_unref(&hw_frames_ref);
		return err;
	}

	ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
	if (!ctx->hw_frames_ctx)
		err = AVERROR(ENOMEM);

	av_buffer_unref(&hw_frames_ref);

	return err;
}



H264_Encoder::H264_Encoder(h264_encoder_callback frameCallback, void* user) 
  :codec(NULL)
  ,c(NULL)
  ,fp(NULL)
  ,sw_frame(nullptr)
  ,cb_frame(frameCallback)
  ,cb_user(user)

{
   avcodec_register_all();


    enum AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
    int pass, fail, skip, err;

    pass = fail = skip = 0;
    while (1) {
        type = av_hwdevice_iterate_types(type);
        if (type == AV_HWDEVICE_TYPE_NONE)
            break;

        err = test_device_type(type);
        if (err == 0) {
            ++pass;
            avcodec_hw_type = type;
        } else if (err < 0)
            ++fail;
        else
            ++skip;


    }

    if (pass)
        ret = av_hwdevice_ctx_create(&avcodec_hw_device_ctx, avcodec_hw_type,
            NULL, NULL, 0);
    if (ret < 0) {
        printf("avcodec: Failed to create HW device \n");

        return;
    }
    

  
}

H264_Encoder::~H264_Encoder() {


  if (pkt) {
    av_packet_free(&pkt);

    pkt = NULL;
  }

  if(c) {
   // avcodec_close(c);
    //av_free(c);
    avcodec_free_context(&c);
    c = NULL;
  }

  if(sw_frame) {
    av_frame_free(&sw_frame);
    sw_frame = NULL;
  }

  
  if(hw_frame) {
    av_frame_free(&hw_frame);
    hw_frame = NULL;
  }

  

  if(fp) {
    fclose(fp);
    fp = NULL;
  }

  cb_frame = NULL;
  cb_user = NULL;
  
  
  
  if (avcodec_hw_device_ctx)
     av_buffer_unref(&avcodec_hw_device_ctx);

}


bool H264_Encoder::load(std::string filename, int fps, int width, int height) {
    

    codec = avcodec_find_encoder_by_name("h264_nvenc");
       
    if (!codec) {
       // fprintf(stderr, "Codec not found\n");
      //  exit(1);
   
        codec = avcodec_find_encoder_by_name("libx264");
        if (!codec) {
            fprintf(stderr, "Codec not found\n");
            exit(1);
        }
   
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

    AVRational tb;
    tb.num = 1;
    tb.den = fps;
    c->time_base = tb;  //arvind
   
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
    c->gop_size = fps;
  //  c->max_b_frames = 1;
    c->max_b_frames = 0;
    c->pix_fmt = avcodec_sw_pix_fmt;
   
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


    if (avcodec_hw_type == AV_HWDEVICE_TYPE_CUDA) {
        int err = set_hwframe_ctx(c, avcodec_hw_device_ctx,
                width, height);
        if (err < 0) {

            printf("avcodec: encode: Failed to set"
                    " hwframe context.\n");
            return false;
        }
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

    printf(" opened %s\n", filename.c_str());

    pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "could not allocate the packet\n");
        exit(1);
    }





    sw_frame = av_frame_alloc();
    if (!sw_frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }


    if (avcodec_hw_type == AV_HWDEVICE_TYPE_CUDA) {
        hw_frame = av_frame_alloc();
        if (!hw_frame) {
            return false;
        }


        //frame->format = c->pix_fmt;
        sw_frame->width = c->width;
        sw_frame->height = c->height;

        sw_frame->format = avcodec_sw_pix_fmt;
        av_frame_get_buffer(sw_frame, 0);


        int err;
        if ((err = av_hwframe_get_buffer(c->hw_frames_ctx, hw_frame, 0)) < 0) {
            printf("avcodec: encode: Error code:\n");
            return false;
        }

        if (!hw_frame->hw_frames_ctx) {

            return false;
        }


    } else {
        sw_frame->format = c->pix_fmt;
        sw_frame->width = c->width;
        sw_frame->height = c->height;

        ret = av_frame_get_buffer(sw_frame, 32);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate the video frame data\n");
            exit(1);
        }

    }
    
  
    
    

  return true;
}


void H264_Encoder::encodeFrame(uint8_t* ydata, int ysize, uint8_t* udata, int usize, uint8_t* vdata, int vsize) {

    av_init_packet(pkt);
    pkt->data = NULL;    // packet data will be allocated by the encoder
    pkt->size = 0;

    fflush(stdout);

    /* make sure the frame data is writable */
    ret = av_frame_make_writable(sw_frame);
    if (ret < 0)
        exit(1);

    /* prepare a dummy image */
    /* Y */

   sw_frame->data[0]  = ydata;
   
   sw_frame->data[1]  = udata;
   
   sw_frame->data[2]  = vdata;
    
    
   sw_frame->linesize[0] = ysize;
   sw_frame->linesize[1] = usize;
   sw_frame->linesize[2] =vsize;
            
   
   sw_frame->pts = ++frameCount;
    
    
   exit(0); // TOB done
    

                
    

    /* encode the image */
    ret = avcodec_encode_video2(c, pkt, sw_frame, &got_output);
    if (ret < 0) {
        fprintf(stderr, "Error encoding frame\n");
        exit(1);
    }

    if (got_output) {
        printf("Write frame %3d (size=%5d)\n", frameCount, pkt->size);
        fwrite(pkt->data, 1, pkt->size, fp);
        av_packet_unref(pkt);
    }
    

    /* get the delayed frames */
    
}






void H264_Encoder::encodeFrame() {

    av_init_packet(pkt);
    pkt->data = NULL; // packet data will be allocated by the encoder
    pkt->size = 0;

    fflush(stdout);

    /* make sure the frame data is writable */
    ret = av_frame_make_writable(sw_frame);
    if (ret < 0)
        exit(1);

    sw_frame->pts = ++frameCount;
    /* prepare a dummy image */
    /* Y */
    for (int y = 0; y < c->height; y++) {
        for (x = 0; x < c->width; x++) {
            sw_frame->data[0][y * sw_frame->linesize[0] + x] = x + y + frameCount * 3;
        }
    }

    /* Cb and Cr */
    for (int y = 0; y < c->height / 2; y++) {
        for (x = 0; x < c->width / 2; x++) {
            sw_frame->data[1][y * sw_frame->linesize[1] + x] = 128 + y + frameCount * 2;
            sw_frame->data[2][y * sw_frame->linesize[2] + x] = 64 + x + frameCount * 5;
        }
    }



    if (avcodec_hw_type == AV_HWDEVICE_TYPE_CUDA) {

        int err;

        if ((err = av_hwframe_transfer_data(hw_frame, sw_frame, 0)) < 0) {
            printf("avcodec: encode: Error while transferring"
                    " frame data to surface."
                    "Error code: \n");
            return;
        }

        av_frame_copy_props(hw_frame, sw_frame);
    }


    /* encode the image */
    /* ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
if (ret < 0) {
    fprintf(stderr, "Error encoding frame\n");
    exit(1);
}

if (got_output) {
    printf("Write frame %3d (size=%5d)\n", frameCount, pkt.size);
    fwrite(pkt.data, 1, pkt.size, fp);
    av_packet_unref(&pkt);
}*/

    if (avcodec_hw_type == AV_HWDEVICE_TYPE_CUDA) {
        ret = avcodec_send_frame(c, hw_frame);
    } else {
        ret = avcodec_send_frame(c, sw_frame);
    }

    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(c, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        printf("Write frame %3d (size=%5d)\n", frameCount, pkt->size);
        fwrite(pkt->data, 1, pkt->size, fp);
        av_packet_unref(pkt);
    }


    /* get the delayed frames */
    
}

