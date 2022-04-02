/*

  H264_Encoder
  ---------------------------------------

  Example that shows how to use the libav parser system. This class forces a 
  H264 parser and codec. You use it by opening a file that is encoded with x264 
  using the `load()` function. You can pass the framerate you want to use for playback.
  If you don't pass the framerate, we will detect it as soon as the parser found 
  the correct information.

  After calling load(), you can call readFrame() which will read a new frame when
  necessary. It will also make sure that it will read enough data from the buffer/file
  when there is not enough data in the buffer.

  `readFrame()` will trigger calls to the given `h264_decoder_callback` that you pass
  to the constructor. 

 */


#ifndef H264_ENCODER_H
#define H264_ENCODER_H

#define H264_INBUF_SIZE 16384                                                           /* number of bytes we read per chunk */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#if 0
#include <tinylib.h>
#endif

extern "C" {
#include <libavcodec/avcodec.h>

#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>

}

typedef void(*h264_encoder_callback)(AVFrame* frame, AVPacket* pkt, void* user);         /* the decoder callback, which will be called when we have decoded a frame */

class H264_Encoder {

 public:
  H264_Encoder(h264_encoder_callback frameCallback, void* user);                         /* pass in a callback function that is called whenever we decoded a video frame, make sure to call `readFrame()` repeatedly */
  ~H264_Encoder();                                                                       /* d'tor, cleans up the allocated objects and closes the codec context */
  bool load(std::string filepath, int fps, int width, int height);     
  /* load a video file which is encoded with x264 */
  ///bool readFrame();                                                                      /* read a frame if necessary */

   void encodeFrame(uint8_t* ydata, int ysize, uint8_t* udata, int usize, uint8_t* vdata, int vsize);
   
   void encodeFrame();
    
 private:
 // bool update(bool& needsMoreBytes);                                                     /* internally used to update/parse the data we read from the buffer or file */
 // int readBuffer();                                                                      /* read a bit more data from the buffer */
  //void encodeFrame(uint8_t* data, int size);  
     /* decode a frame we read from the buffer */

 
         
     
 public:
  AVCodec* codec{nullptr};                                                                        /* the AVCodec* which represents the H264 decoder */
  AVCodecContext* c;                                                         /* the context; keeps generic state */
                                                      /* parser that is used to decode the h264 bitstream */
                                                                 /* will contain a decoded picture */
  //int8_t inbuf[H264_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];                         /* used to read chunks from the file */
  FILE* fp;                                                                              /* file pointer to the file from which we read the h264 data */
  //int frame;                                                                             /* the number of decoded frames */
  h264_encoder_callback cb_frame;                                                        /* the callback function which will receive the frame/packet data */
  void* cb_user;                                                                         /* the void* with user data that is passed into the set callback */
 
  
  
   ///const char *filename, *codec_name;
    //const AVCodec *codec;
    //AVCodecContext *c= NULL;
    int frameCount{0};
    int  ret, x, y, got_output;
    //FILE *f;
    AVFrame *frame;
    AVPacket* pkt{NULL};
  /* buffer we use to keep track of read/unused bitstream data */
};

#endif
