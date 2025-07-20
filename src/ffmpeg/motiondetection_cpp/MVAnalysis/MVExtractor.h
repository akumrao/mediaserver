//extern "C" {
//#include <libavutil/motion_vector.h>
//#include <libavformat/avformat.h>
// //#ifdef _MV_DEBUG_
//  #include<libavutil/pixdesc.h>
//  #include<libswscale/swscale.h>
// // #endif
//}


#include "ffmpeg_h.h"

#define H264_INBUF_SIZE 16384   

#include "MVDetector.h"
#include "Contour.h"

#ifndef H264_MVE_H
#define H264_MVE_H

class MVExtractor {
public:

    MVExtractor(const char *src_filename);

    ~MVExtractor();

 
   // AVCodecContext *video_dec_ctx{NULL};
    //AVStream *video_stream{NULL};
    //const char *src_filename{NULL};

    //int video_stream_idx{-1};
    AVFrame *frame{NULL};
    //AVPacket pkt;
    int video_frame_count{0};

    int dst_width{0};
    int dst_height{0};
    void extract();
    
    static void CallBackFunc(int event, int x, int y, int flags, void* userdata) ;

private:

    ContDetector *mv_detector{nullptr};
    
    

    //int open_codec_context(AVFormatContext *fmt_ctx, enum AVMediaType type);

    //int decode_packet(int *got_frame, int cached);
    
   // bool update(bool& needsMoreBytes);
    int decodeFrame(uint8_t* data, int size);
    int read(const char *src_filename);
    


 #ifdef _MV_DEBUG_

#endif
   
   
  AVCodecParserContext* parser;                                                          /* parser that is used to decode the h264 bitstream */
                                                                 /* will contain a decoded picture */
  //uint8_t inbuf[H264_INBUF_SIZE + 32];                         /* used to read chunks from the file */
  //FILE* fp;                                                                             /* file pointer to the file from which we read the h264 data */
  FILE *file; 
  //std::vector<uint8_t> buffer; 
  
  AVCodecContext *dec_ctx{NULL};     
  
  bool first_time{true};

};


#endif
