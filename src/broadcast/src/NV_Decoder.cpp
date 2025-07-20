
#include "NV_Decoder.h"

namespace base {
    namespace web_rtc {

NV_Decoder::NV_Decoder(st_track  &trackInfo, AVCodecID &codec_id) :H264_Decoder(trackInfo, codec_id)
{
  
   
}

NV_Decoder::~NV_Decoder() {

    
    
}


void NV_Decoder::pushQueue( AVFrame *frame)
{

   
    if (frame->format == AV_PIX_FMT_CUDA) 
    {
         AVFrame  *sw_frame = NULL;
        
         if (!(sw_frame = av_frame_alloc())) {
            fprintf(stderr, "Can not alloc frame\n");
            exit(0);
        }
         
        // retrieve data from GPU to CPU 
        if (( av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
            fprintf(stderr, "Error transferring the data to system memory\n");
            exit(0);
        }
        if(frameQueue.size() > 0)
        frameQueue.back().push_front(sw_frame);
        
        av_frame_free(&frame);
    }
    else
    {
       if(frameQueue.size() > 0)
         frameQueue.back().push_front(frame);
    }
    
    popQueue( );

}

void NV_Decoder::popQueue( )
{

    if(frameQueue.size() > 1 )
    {
        std::list<AVFrame* > &list =frameQueue.front();
          
        //SInfo << list.size();
        
        AVFrame *frame =list.front();

        //int width = frame->width;
       // int height =  frame->height;
      
        list.pop_front();
        
        if(! list.size())
            frameQueue.pop();
        
        
        frame->pts = vframecount;
   
        cb_frame(codec_context, frame );
       
        
        
        //av_frame_free(&frame);
        
        delayFrame( );

   
    }
    
}
}}