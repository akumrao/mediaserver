
#include "MVDetectorBase.h"
#include "framefilter.h"


#include "json/configuration.h"
#include "ffmpeg_h.h"

#ifndef H264_MVE_H
#define H264_MVE_H


//#define MOTIONCOUNT 2

//#define CLIPSIZE  5

namespace base {
namespace fmp4 {
    
    
class FragMP4MuxFrameFilter;

class MVExtractor :public FrameFilter {
public:

    MVExtractor(const char *name, std::string & cam,  FrameFilter *txt, FrameFilter *muxer, FrameFilter *fileFilter, FrameFilter *next = NULL) ;
   // MVExtractor(const char *src_filename);

    ~MVExtractor();
    
    int Init( BasicFrame *bs);
    
    
   // AVFormatContext     *av_format_context;
    AVPacket             *avpkt;
    AVDictionary        *av_dict;
    
     AVCodecContext *av_codec_context;
     
   // AVFormatContext *fmt_ctx{NULL};
   // AVCodecContext *video_dec_ctx{NULL};
   // AVStream *video_stream{NULL};
    //const char *src_filename{NULL};

    //int video_stream_idx{0};
    AVFrame *frame{NULL};
    //AVPacket pkt;
    uint64_t video_frame_count{0};
    int dst_width{0};
    int dst_height{0};

    //void extract();
    bool extract( AVPacket *pk ,   short unsigned slice_type, BasicFrame *bs );
    
    int continueMotion{0};
       
    int iFrameCount{0};
    
    std::string  cam;
    
    BasicFrame sps;
    BasicFrame pps;
    
    std::vector< BasicFrame > priClip;
    
    std::queue<int> priClipGop; 
    
    ///BasicFrame  basicframe; 
        
    
    std::string h264;
    
    TextFrame txtFrame;
      
    int send();
private:

    MVDetectorBase *mv_detector{nullptr};
    
    FrameFilter *txt;
    
    FrameFilter *muxer;
    
    FrameFilter *fileFilter;
        
    FragMP4MuxFrameFilter *fragmp4_muxer;
    FileFrameFilter *fileSave;
    
    
    base::cnfg::Configuration config;

    //int open_codec_context(AVFormatContext *fmt_ctx, enum AVMediaType type);

   // int decode_packet(int *got_frame, int cached);

   // int read(const char *src_filename);

    Settings::MDConfig mdConfig;
    
    protected:
    void go(Frame *frame);
    
    bool initialized{false};
      
    bool parseH264Header();
    
   #if FPSONE


    bool foundGop{false};
    int gop{0};
    int pFrame{0};
    int incPframe{0};
   
  #endif

  int gop{0};


};

}
}
#endif
