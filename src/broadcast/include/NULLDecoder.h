
#pragma once



#include "webrtc/rawVideoFrame.h"
#include  "muxframe.h"

#include  <queue>

extern "C" {
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
};

#include <functional>


namespace base {
namespace web_rtc {
    


class NULLDecoder
{
public:


    NULLDecoder(st_track  &trackInfo);
    ~NULLDecoder();
    
    uint64_t startStreaming{0};
    uint64_t vframecount{0};
    
   
     
    void runNULLEnc(unsigned char *buffer, int size, AVPictureType pict_type); 
    
    void resetTimer();
    
    void delayFrame( );
    
    stFrame *qframe{nullptr};
    
    std::function<void(stFrame* frame) > cb_frame;
    
    
    int  width{0};
    int height{0};
    int fps;
    
    
private:
    std::vector< uint8_t> m_sps;
    std::vector< uint8_t> m_pps;
    
    uint64_t vdelay{0};
    
    st_track  &trackInfo;
      
        
};



}//ns web_rtc
}//base
