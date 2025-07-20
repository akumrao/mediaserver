#include "MVExtractor.h"

#ifdef _MV_DEBUG_
#include<opencv2/highgui.hpp>
char run_mode = 'r';
#endif


int main(int argc, char **argv)
{
    //int ret = 0, got_frame;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <video rtsp or mp4> <mode run or pause>[p][r] \n", argv[0]);
        exit(1);
    }
  
    
    const char *src_filename = argv[1];
    
    if(argc ==3) 
    run_mode = *argv[2];

#ifdef TRACE    
    av_log_set_level(AV_LOG_TRACE);
#endif 

    av_register_all();
    
    MVExtractor  mve(src_filename );
    
    mve.extract();
    
    return 0;
}
