/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fmp4.h
 * Author: root
 *
 * Created on June 8, 2021, 10:48 PM
 */

#ifndef FFParse_H
#define FFParse_H

#include "base/thread.h"
#include <string>
#include <vector>
#include "muxer.h"

// #include "net/netInterface.h"
// #include "http/HttpsClient.h"



extern "C" {
 
// #include <libavutil/avassert.h>    
 #include <avcodec.h>
// #include <libavutil/opt.h>
// #include <libswscale/swscale.h>
// #ifdef HAVE_FFMPEG_SWRESAMPLE
// #include <libswresample/swresample.h>
// #else
// #include <libavresample/avresample.h>
// #endif
}

 


namespace base {
namespace fmp4 {
 #if 0 
   // a wrapper around a single output AVStream
typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    //AVFrame *tmp_frame;

    //float t, tincr, tincr2;

   // struct SwsContext *sws_ctx;
   // struct SwrContext *swr_ctx;
    
    FILE *in_file;
//    OutputStream()
//    {
//        in_file = nullptr;
//    }
} OutputStream;
#endif



 class FFParse: public Thread
 {
     
     
 public:
  
    
     FFParse( base::fmp4::ReadMp4 *conn, const char* audioFile, const char*  videofile );
     
     ~FFParse( );
     
     int fmp4( const char *in_filename, const char *out_filename =nullptr, bool fragmented_mp4_options=true);
          
   //virtual void start() override
   // virtual void stop() override;
     void run() override;
     
     std::vector<uint8_t> outputData;
     bool looping{true};
     
    
     /// Video Begin 
    bool parseH264Header();
    
    void parseH264Content();
    FILE *fileVideo;
    BasicFrame        basicvideoframe;  ///< Data is being copied into this frame
    /// Video End
      
    /// Audio Begin 
    
    FILE *fileAudio;
    bool parseAACHeader();
    void parseAACContent();
     AVCodecContext *audioContext= NULL;
    
    void reset();
    void restart(bool mute);
    int startAudio( );
      
    long get_nal_size(uint8_t *buf, long size,  uint8_t **poutbuf, int *poutbuf_size);
      
    
   // net::ClientConnecton *conn;    
     
    BasicFrame        basicaudioframe;  ///< Data is being copied into this frame
   /// Audio End 
    
    long int startTime{0};
    int stream_index{0};
    void parseMuxContent();
    
    void mediaContent(std::string mediaContent);
       
 private:
     
    std::atomic< bool > resetParser { false };
    std::atomic< bool > mute { false };

    std::atomic< bool > keeprunning { true };

    DummyFrameFilter fragmp4_filter;
    FragMP4MuxFrameFilter fragmp4_muxer;
    InfoFrameFilter info;
    TextFrameFilter txt;
    
    //std::string fileName;
    
 #if 0   
    /* Add an output stream. */
  void add_stream(OutputStream *ost, AVFormatContext *oc,
                       AVCodec **codec,
                       enum AVCodecID codec_id);


     AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples);
     void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg);
     AVFrame *get_audio_frame(OutputStream *ost);
     int write_audio_frame(AVFormatContext *oc, OutputStream *ost);
    void close_stream(AVFormatContext *oc, OutputStream *ost);
#endif
    

 };
 
}
}

#endif /* FFParse_H */

